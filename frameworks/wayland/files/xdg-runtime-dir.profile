# Respect runtime directories supplied by PAM or an existing desktop session.
if [ -z "${XDG_RUNTIME_DIR:-}" ]; then
	export XDG_RUNTIME_DIR="/tmp/run/$LOGNAME"
	mkdir -m 0700 -p "$XDG_RUNTIME_DIR"
fi
