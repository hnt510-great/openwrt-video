// SPDX-License-Identifier: MIT
/* Per-peer PipeWire streams for Weston's authenticated shared RDP desktop.
 * Neither PipeWire nor network audio callbacks run on the compositor thread.
 */
#include "config.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <freerdp/server/rdpsnd.h>
#include <freerdp/server/audin.h>
#include <freerdp/server/server-common.h>
#include <libweston/libweston.h>
#include "rdpaudio.h"

/* Less than 200 ms of stereo PCM, with whole-frame overflow dropping. */
#define AUDIO_QUEUE 32768
#define AUDIO_PACKET 1920
struct rdp_audio {
	struct pw_thread_loop *loop;
	struct pw_stream *stream;
	pthread_mutex_t mutex;
	pthread_cond_t wake;
	pthread_t worker;
	bool worker_started;
	bool loop_started;
	atomic_bool stopping;
	atomic_bool ready;
	bool capture;
	HANDLE vcm;
	RdpsndServerContext *out;
	audin_server_context *in;
	AUDIO_FORMAT pcm;
	uint8_t queue[AUDIO_QUEUE];
	size_t read_pos, write_pos, queued;
};
static pthread_once_t pw_once = PTHREAD_ONCE_INIT;
static atomic_uint source_id;

static void audio_init(void) { pw_init(NULL, NULL); }

static void queue_write(struct rdp_audio *a, const void *data, size_t size)
{
	size_t first;

	if (size > AUDIO_QUEUE || pthread_mutex_trylock(&a->mutex))
		return;
	if (size <= AUDIO_QUEUE - a->queued) {
		first = SPA_MIN(size, AUDIO_QUEUE - a->write_pos);
		memcpy(a->queue + a->write_pos, data, first);
		memcpy(a->queue, (const uint8_t *)data + first, size - first);
		a->write_pos = (a->write_pos + size) % AUDIO_QUEUE;
		a->queued += size;
		pthread_cond_signal(&a->wake);
	}
	pthread_mutex_unlock(&a->mutex);
}

/* Caller holds mutex. */
static size_t queue_read(struct rdp_audio *a, void *data, size_t size)
{
	size_t first;

	size = SPA_MIN(size, a->queued);
	first = SPA_MIN(size, AUDIO_QUEUE - a->read_pos);
	memcpy(data, a->queue + a->read_pos, first);
	memcpy((uint8_t *)data + first, a->queue, size - first);
	a->read_pos = (a->read_pos + size) % AUDIO_QUEUE;
	a->queued -= size;
	return size;
}

static void stream_process(void *data)
{
	struct rdp_audio *a = data;
	struct pw_buffer *pb;
	struct spa_data *d;
	size_t size, offset;

	pb = pw_stream_dequeue_buffer(a->stream);
	if (!pb)
		return;
	if (!pb->buffer->n_datas)
		goto done;
	d = &pb->buffer->datas[0];
	if (!d->data || !d->chunk || !d->maxsize)
		goto done;
	if (!a->capture) {
		offset = d->chunk->offset % d->maxsize;
		size = SPA_MIN(d->chunk->size, d->maxsize - offset);
		size -= size % 4;
		if (atomic_load(&a->ready) && !(d->chunk->flags & SPA_CHUNK_FLAG_CORRUPTED))
			queue_write(a, (uint8_t *)d->data + offset, size);
	} else {
		size = SPA_MIN((size_t)d->maxsize, (size_t)pb->requested * 2);
		if (!size)
			size = SPA_MIN(d->maxsize, (uint32_t)AUDIO_PACKET);
		size -= size % 2;
		memset(d->data, 0, size);
		if (!pthread_mutex_trylock(&a->mutex)) {
			queue_read(a, d->data, size);
			pthread_mutex_unlock(&a->mutex);
		}
		d->chunk->offset = 0;
		d->chunk->stride = 2;
		d->chunk->size = size;
		pb->size = size / 2;
	}
 done:
	pw_stream_queue_buffer(a->stream, pb);
}

static const struct pw_stream_events stream_events = {
	PW_VERSION_STREAM_EVENTS,
	.process = stream_process,
};

static bool audio_connect(struct rdp_audio *a)
{
	uint8_t buffer[1024];
	struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
	struct spa_audio_info_raw info = { .format = SPA_AUDIO_FORMAT_S16_LE,
		.rate = 48000, .channels = a->capture ? 1 : 2 };
	const struct spa_pod *param;
	struct pw_properties *props;
	char name[64];
	int rc;

	pthread_once(&pw_once, audio_init);
	a->loop = pw_thread_loop_new("weston-rdp-audio", NULL);
	if (!a->loop || pw_thread_loop_start(a->loop) < 0)
		return false;
	a->loop_started = true;
	props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio",
		PW_KEY_MEDIA_CATEGORY, a->capture ? "Playback" : "Capture",
		PW_KEY_MEDIA_ROLE, "Communication", NULL);
	if (!props)
		return false;
	if (a->capture) {
		snprintf(name, sizeof(name), "rdp-microphone-%u", atomic_fetch_add(&source_id, 1));
		pw_properties_set(props, PW_KEY_NODE_NAME, name);
		pw_properties_set(props, PW_KEY_NODE_DESCRIPTION, "RDP client microphone");
		pw_properties_set(props, PW_KEY_MEDIA_CLASS, "Audio/Source");
		pw_properties_set(props, PW_KEY_NODE_VIRTUAL, "true");
		pw_properties_set(props, PW_KEY_NODE_ALWAYS_PROCESS, "true");
		info.position[0] = SPA_AUDIO_CHANNEL_MONO;
	} else {
		pw_properties_set(props, "stream.capture.sink", "true");
		pw_properties_set(props, PW_KEY_NODE_NAME, "weston-rdp-playback");
		info.position[0] = SPA_AUDIO_CHANNEL_FL;
		info.position[1] = SPA_AUDIO_CHANNEL_FR;
	}
	pw_thread_loop_lock(a->loop);
	a->stream = pw_stream_new_simple(pw_thread_loop_get_loop(a->loop),
		"Weston RDP audio", props, &stream_events, a);
	if (!a->stream) {
		pw_thread_loop_unlock(a->loop);
		return false;
	}
	param = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &info);
	rc = pw_stream_connect(a->stream, a->capture ? PW_DIRECTION_OUTPUT : PW_DIRECTION_INPUT,
		PW_ID_ANY, PW_STREAM_FLAG_MAP_BUFFERS |
		(a->capture ? 0 : PW_STREAM_FLAG_AUTOCONNECT), &param, 1);
	pw_thread_loop_unlock(a->loop);
	return rc >= 0;
}

static void out_activated(RdpsndServerContext *out)
{
	struct rdp_audio *a = out->data;
	/* PCM is interoperable with ordinary RDP desktop clients. */
	const UINT16 tags[] = { WAVE_FORMAT_PCM };
	for (size_t t = 0; t < SPA_N_ELEMENTS(tags); t++) {
		for (size_t i = 0; i < out->num_client_formats; i++) {
			if (out->client_formats[i].wFormatTag != tags[t])
				continue;
			for (size_t j = 0; j < out->num_server_formats; j++) {
				if (audio_format_compatible(&out->server_formats[j], &out->client_formats[i]) &&
				    out->SelectFormat(out, (UINT16)i) == CHANNEL_RC_OK) {
					const AUDIO_FORMAT *fmt = &out->client_formats[i];

					weston_log("RDP playback format: tag=0x%04x rate=%u channels=%u bitrate=%u\n",
						(unsigned int)fmt->wFormatTag, (unsigned int)fmt->nSamplesPerSec,
						(unsigned int)fmt->nChannels, (unsigned int)fmt->nAvgBytesPerSec * 8);
					atomic_store(&a->ready, true);
					return;
				}
			}
		}
	}
}

static UINT in_data(audin_server_context *in, const SNDIN_DATA *data)
{
	struct rdp_audio *a = in->userdata;
	const AUDIO_FORMAT *fmt = audin_server_get_negotiated_format(in);
	size_t size = Stream_GetRemainingLength(data->Data);
	/* AUDIN delivers encoded packets. Only advertise PCM for microphone input. */
	if (!fmt || fmt->wFormatTag != WAVE_FORMAT_PCM || fmt->nSamplesPerSec != 48000 ||
	    fmt->nChannels != 1 || fmt->wBitsPerSample != 16 || size % 2)
		return ERROR_INVALID_DATA;
	queue_write(a, Stream_Pointer(data->Data), size);
	return CHANNEL_RC_OK;
}

static void *audio_worker(void *data)
{
	struct rdp_audio *a = data;
	uint8_t packet[AUDIO_PACKET];
	struct timespec now;
	UINT16 timestamp;

	if (a->capture) {
		while (!atomic_load(&a->stopping)) {
			BYTE state = WTSVirtualChannelManagerGetDrdynvcState(a->vcm);

			if (state == DRDYNVC_STATE_READY) {
				if (!a->in->Open(a->in))
					weston_log("RDP microphone channel unavailable\n");
				break;
			}
			if (state == DRDYNVC_STATE_FAILED)
				break;
			usleep(100000);
		}
		return NULL;
	}
	while (!atomic_load(&a->stopping)) {
		pthread_mutex_lock(&a->mutex);
		while (a->queued < sizeof(packet) && !atomic_load(&a->stopping))
			pthread_cond_wait(&a->wake, &a->mutex);
		if (atomic_load(&a->stopping)) {
			pthread_mutex_unlock(&a->mutex);
			break;
		}
		queue_read(a, packet, sizeof(packet));
		pthread_mutex_unlock(&a->mutex);
		clock_gettime(CLOCK_MONOTONIC, &now);
		timestamp = (UINT16)((uint64_t)now.tv_sec * 1000 + now.tv_nsec / 1000000);
		if (a->out->SendSamples(a->out, packet, sizeof(packet) / 4, timestamp) != CHANNEL_RC_OK) {
			atomic_store(&a->ready, false);
			break;
		}
	}
	return NULL;
}

void weston_rdp_audio_teardown(void *data)
{
	struct rdp_audio *a = data;

	if (!a)
		return;
	atomic_store(&a->stopping, true);
	pthread_mutex_lock(&a->mutex);
	pthread_cond_broadcast(&a->wake);
	pthread_mutex_unlock(&a->mutex);
	if (a->worker_started)
		pthread_join(a->worker, NULL);
	if (a->out)
		rdpsnd_server_context_free(a->out);
	if (a->in)
		audin_server_context_free(a->in);
	if (a->loop) {
		if (a->loop_started)
			pw_thread_loop_stop(a->loop);
		if (a->stream)
			pw_stream_destroy(a->stream);
		pw_thread_loop_destroy(a->loop);
	}
	pthread_cond_destroy(&a->wake);
	pthread_mutex_destroy(&a->mutex);
	free(a);
}

static void *audio_setup(struct weston_compositor *c, void *vcm, bool capture)
{
	struct rdp_audio *a = calloc(1, sizeof(*a));
	(void)c;
	if (!a)
		return NULL;
	a->vcm = vcm;
	a->capture = capture;
	atomic_init(&a->stopping, false);
	atomic_init(&a->ready, false);
	if (pthread_mutex_init(&a->mutex, NULL)) {
		free(a);
		return NULL;
	}
	if (pthread_cond_init(&a->wake, NULL)) {
		pthread_mutex_destroy(&a->mutex);
		free(a);
		return NULL;
	}
	a->pcm = (AUDIO_FORMAT) { .wFormatTag = WAVE_FORMAT_PCM,
		.nChannels = capture ? 1 : 2, .nSamplesPerSec = 48000,
		.nAvgBytesPerSec = capture ? 96000 : 192000,
		.nBlockAlign = capture ? 2 : 4, .wBitsPerSample = 16 };
	if (!audio_connect(a))
		goto fail;
	if (capture) {
		a->in = audin_server_context_new(vcm);
		if (!a->in)
			goto fail;
		a->in->userdata = a;
		a->in->Data = in_data;
		if (!audin_server_set_formats(a->in, 1, &a->pcm))
			goto fail;
	} else {
		a->out = rdpsnd_server_context_new(vcm);
		if (!a->out)
			goto fail;
		a->out->data = a;
		a->out->src_format = &a->pcm;
		a->out->latency = 20;
		a->out->server_formats = calloc(2, sizeof(AUDIO_FORMAT));
		if (!a->out->server_formats)
			goto fail;
		a->out->num_server_formats = 2;
		a->out->server_formats[0] = a->pcm;
		a->out->server_formats[1] = a->pcm;
		a->out->server_formats[1].nSamplesPerSec = 44100;
		a->out->server_formats[1].nAvgBytesPerSec = 176400;
		a->out->Activated = out_activated;
		if (!a->out->num_server_formats || a->out->Initialize(a->out, TRUE) != CHANNEL_RC_OK)
			goto fail;
	}
	if (pthread_create(&a->worker, NULL, audio_worker, a))
		goto fail;
	a->worker_started = true;
	return a;
 fail:
	weston_log("RDP %s audio unavailable; desktop sharing continues\n", capture ? "microphone" : "playback");
	weston_rdp_audio_teardown(a);
	return NULL;
}

void *weston_rdp_audio_out_setup(struct weston_compositor *c, void *vcm)
{
	return audio_setup(c, vcm, false);
}
void *weston_rdp_audio_in_setup(struct weston_compositor *c, void *vcm)
{
	return audio_setup(c, vcm, true);
}
