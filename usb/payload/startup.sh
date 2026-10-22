#!/bin/sh
#
# Clinton Electronics Shadow DVR (CE-R4S / LT400)
# Custom Persistence Startup Script (/mtd/startup.sh)
#

# 1. Start root telnet daemon on standard port 23
/usr/sbin/telnetd -l /bin/sh &

# 2. Link RAM-based keyframes to web server document root for instant snapshots
ln -sf /tmp/live_0.h264 /secube/web/live_0.h264
ln -sf /tmp/live_1.h264 /secube/web/live_1.h264
if [ -e /tmp/live_2.h264 ]; then
    ln -sf /tmp/live_2.h264 /secube/web/live_2.h264
fi
if [ -e /tmp/live_3.h264 ]; then
    ln -sf /tmp/live_3.h264 /secube/web/live_3.h264
fi

# 3. Start standalone micro RTSP daemon in background
(sleep 3 && /mtd/bin/astel_rtspd >/tmp/astel_rtspd.log 2>&1 &) &
