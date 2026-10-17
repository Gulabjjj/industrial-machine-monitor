#!/bin/sh
set -e
export XDG_RUNTIME_DIR=/tmp/runtime-root
mkdir -p "$XDG_RUNTIME_DIR" && chmod 700 "$XDG_RUNTIME_DIR"

Xvfb :0 -screen 0 "${SCREEN_RES:-1280x800x24}" -nolisten tcp &
sleep 1

IndustrialMachineMonitor &

VNC_ARGS="-display :0 -forever -shared -rfbport 5900 -localhost -quiet"
if [ -n "$VNC_PASSWORD" ]; then
    x11vnc -storepasswd "$VNC_PASSWORD" /tmp/vncpass >/dev/null 2>&1
    x11vnc $VNC_ARGS -rfbauth /tmp/vncpass &
else
    x11vnc $VNC_ARGS -nopw &
fi
sleep 1

# Serve the noVNC web client on the port Render provides
exec websockify --web /usr/share/novnc "${PORT:-10000}" localhost:5900
