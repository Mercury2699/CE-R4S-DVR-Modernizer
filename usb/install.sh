#!/bin/sh
#
# install.sh - Manual Installer for Clinton Electronics Shadow DVR
# Run this script directly on the DVR via Telnet or Serial console:
#   cd /mnt/usb && ./install.sh
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PAYLOAD="$SCRIPT_DIR/payload"

if [ ! -d "$PAYLOAD" ]; then
    echo "[-] Error: payload directory not found in $SCRIPT_DIR"
    exit 1
fi

echo "=== Installing Clinton Shadow DVR Toolkit ==="

mkdir -p /mtd/bin /mtd/web

echo "[1/5] Stopping existing RTSP server if running..."
killall astel_rtspd 2>/dev/null || true

echo "[2/5] Copying binaries to /mtd/bin..."
cp -f "$PAYLOAD/astel_rtspd" /mtd/bin/astel_rtspd
cp -f "$PAYLOAD/dvr_auth" /mtd/bin/dvr_auth
if [ -f "$PAYLOAD/api_status" ]; then
    cp -f "$PAYLOAD/api_status" /mtd/bin/api_status
fi
chmod 755 /mtd/bin/astel_rtspd /mtd/bin/dvr_auth

echo "[3/5] Copying Web Console assets to /mtd/web..."
cp -f "$PAYLOAD/index.cgi" /mtd/web/index.cgi
cp -f "$PAYLOAD/api_status.cgi" /mtd/web/api_status.cgi
cp -f "$PAYLOAD/api_exec.cgi" /mtd/web/api_exec.cgi
cp -f "$PAYLOAD/jmuxer.min.js" /mtd/web/jmuxer.min.js
chmod 755 /mtd/web/index.cgi /mtd/web/api_status.cgi /mtd/web/api_exec.cgi
chmod 644 /mtd/web/jmuxer.min.js

echo "[4/5] Setting up /mtd/startup.sh persistence..."
cp -f "$PAYLOAD/startup.sh" /mtd/startup.sh
chmod 755 /mtd/startup.sh

echo "[5/5] Starting services..."
/usr/sbin/telnetd -l /bin/sh &
ln -sf /tmp/live_0.h264 /secube/web/live_0.h264 2>/dev/null || true
ln -sf /tmp/live_1.h264 /secube/web/live_1.h264 2>/dev/null || true
(/mtd/bin/astel_rtspd >/tmp/astel_rtspd.log 2>&1 &) &

sync
echo "=== Installation Complete! ==="
echo "WebUI:  http://$(ifconfig eth0 | grep 'inet addr' | awk -F: '{print $2}' | awk '{print $1}')/"
echo "RTSP:   rtsp://$(ifconfig eth0 | grep 'inet addr' | awk -F: '{print $2}' | awk '{print $1}'):554/live/0"
