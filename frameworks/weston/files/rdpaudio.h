/* SPDX-License-Identifier: MIT */
#ifndef WESTON_RDP_AUDIO_H
#define WESTON_RDP_AUDIO_H
struct weston_compositor;
void *weston_rdp_audio_out_setup(struct weston_compositor *c, void *vcm);
void *weston_rdp_audio_in_setup(struct weston_compositor *c, void *vcm);
void weston_rdp_audio_teardown(void *data);
#endif
