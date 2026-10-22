# astel_rtspd - Freestanding ARMv4 Micro RTSP Daemon

## 1. Overview
`astel_rtspd` is a high-performance, ultra-lightweight RTSP server written specifically for the Faraday FA526 (ARMv4l) architecture running Linux 2.6.15.

### Key Capabilities:
* **Zero libc dependencies**: Freestanding binary compiled with `-nostdlib`, making direct Linux OABI system calls (`swi 0x900000`). Total binary size is only **13 KB**.
* **512 KB GOP Ring Buffer per channel**: Caches the most recent SPS, PPS, and complete IDR keyframe burst in memory. When a client issues `PLAY`, the GOP burst is pushed immediately, eliminating the 5–8 second delay waiting for the next hardware IDR frame.
* **Strict Monotonic RTP Timestamping**: Prevents H.264 reference frame tearing caused by back-to-back packets with identical timestamps.
* **Low Memory Footprint**: BSS allocation of ~1.2 MB; CPU usage during active 30fps streaming is **< 1%**.
* **RFC 6184 H.264 RTP Packetization**: Supports Single NAL Unit Mode (Packetization Mode 0 / 1) and FU-A fragmentation for packets exceeding MTU (1400 bytes).

---

## 2. Compilation

```bash
# Compile freestanding ARMv4 binary
arm-linux-gnueabi-gcc -march=armv4 -marm -nostdlib -fno-builtin -no-pie -static -O2 -s astel_rtspd.c -o astel_rtspd
```

---

## 3. Usage & Endpoints

Run directly on the DVR:
```sh
/mtd/bin/astel_rtspd &
```

RTSP URLs:
* Channel 1: `rtsp://<dvr-ip>:554/live/0`
* Channel 2: `rtsp://<dvr-ip>:554/live/1`
* Channel 3: `rtsp://<dvr-ip>:554/live/2`
* Channel 4: `rtsp://<dvr-ip>:554/live/3`
