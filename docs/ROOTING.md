# Achieving Root Access & USB Auto-Patching

## 1. Discovery of the Firmware USB Autorun Backdoor

During analysis of the Clinton DVR init scripts in `/etc/rc.d/rc.upgrade`, an undocumented diagnostic & manufacturing upgrade mechanism was discovered:

```sh
# Inside /etc/rc.d/rc.upgrade:
EXT_UPGRADE="rc.upgrade.ext"
...
# When USB mass storage is plugged in:
if [ -f $SOURCE_DIR/$EXT_UPGRADE ]; then
    cp -f $SOURCE_DIR/$EXT_UPGRADE /etc/rc.d/$EXT_UPGRADE
    mv $SOURCE_DIR/$EXT_UPGRADE $SOURCE_DIR/rc.upgrade.tmp
    umount $SOURCE_DIR

    chmod 0755 /etc/rc.d/$EXT_UPGRADE
    /etc/rc.d/$EXT_UPGRADE
    exit 0
fi
```

### How the Exploit Works:
1. When any USB flash drive formatted as FAT32/ext2 is plugged into the DVR, the firmware automatically scans the drive during boot or periodic device polling.
2. If the drive contains a file named `rc.upgrade.ext` in its root directory, the DVR **copies the script to the root filesystem, marks it executable, and runs it with root privileges**.
3. Zero authentication, zero passwords, and zero console interaction are required.

---

## 2. One-Touch USB Installation

Using this mechanism, we can achieve complete, permanent customization on any stock DVR in less than 30 seconds:

1. Copy `rc.upgrade.ext` and the `payload/` directory onto a standard USB flash drive.
2. Plug the USB flash drive into the front or rear USB port of the DVR.
3. Power-cycle or reboot the DVR.
4. The system executes `rc.upgrade.ext`, which:
   * Installs `/mtd/bin/astel_rtspd` (RTSP server)
   * Installs modern HTML5 Web Console in `/mtd/web/`
   * Installs `/mtd/startup.sh` for boot persistence
   * Spawns `/usr/sbin/telnetd -l /bin/sh` on port 23
   * Writes `INSTALL_SUCCESS.txt` to the USB drive
5. You can now unplug the USB drive. The DVR now has full root access and standalone RTSP streaming enabled.
