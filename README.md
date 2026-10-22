# CE-R4S DVR Modernizer 🚀

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Architecture](https://img.shields.io/badge/Architecture-ARMv4%20%7C%20FA526-orange.svg)](docs/HARDWARE.md)
[![Kernel](https://img.shields.io/badge/Kernel-Linux%202.6.15%20OABI-green.svg)](docs/HARDWARE.md)
[![RTSP](https://img.shields.io/badge/RTSP-554%20H.264%20RFC6184-brightgreen.svg)](rtsp/README.md)
[![HomeKit](https://img.shields.io/badge/Apple%20HomeKit-Sub--second%20Streaming-purple.svg)](homeassistant/README.md)

Open-source firmware enhancements, native standalone RTSP streaming daemon, and modern HTML5 Web Console for **Clinton Electronics Shadow Series DVRs** (CE-R4S, LT400 series, and related Astel / Faraday FA526 hardware).

---

## 🌟 Highlights

* **⚡ Zero-Touch USB Auto-Installer**: Plug a formatted USB drive into the DVR and power on. Automatically achieves root privileges, installs all software to `/mtd/`, enables persistent telnet, and configures modern services in ~15 seconds without disassembling or opening the case.
* **🎥 Ultra-Lightweight Native RTSP Daemon (`astel_rtspd`)**:
  * Written in freestanding C (`-nostdlib`, direct Linux OABI system calls).
  * **13 KB** single binary; **< 1% CPU usage**.
  * **512 KB GOP Cache**: Instant sub-second live stream launch on Apple HomeKit / VLC / Web.
  * In-memory 32-bit endianness byte swapping for standard RFC 6184 H.264 streams.
* **💻 Modern Standalone HTML5 Web Console**:
  * Replaces legacy Internet Explorer 6 / ActiveX plugins.
  * Runs on all modern mobile and desktop browsers (iOS Safari, Android Chrome, macOS, Windows).
  * Browser-side hardware-accelerated H.264 decoding with 4:3 aspect ratio enforcement.
  * Real-time hardware telemetry: CPU load, RAM usage, MTD flash health, SMART HDD diagnostics.
  * Administrative Web Shell console with authorization.
* **🏠 Native Smart Home Integration**:
  * 100% stock Home Assistant support (`camera: platform ffmpeg`).
  * Direct Apple HomeKit integration with 0.5s instant feed launch and zero dropped frames.

---

## 📦 Quick Start: One-Touch USB Installation

No serial cables, soldering, or firmware flashing tools needed.

### 1. Download or Build the Package
Download the latest `CE-R4S-Modernizer-v1.0.0-USB.zip` from Releases, or build it locally:
```bash
./build_and_package.sh
```

### 2. Prepare the USB Drive
1. Format any standard USB flash drive as **FAT32** (MBR partition scheme).
2. Extract `CE-R4S-Modernizer-v1.0.0-USB.zip` directly into the **root** of the USB drive:
   ```text
   /Volumes/USB/
   ├── rc.upgrade.ext       <-- Firmware auto-run hook
   ├── install.sh           <-- Manual shell installer
   └── payload/             <-- Native binaries and Web assets
       ├── astel_rtspd
       ├── dvr_auth
       ├── index.cgi
       ├── api_status.cgi
       ├── api_exec.cgi
       ├── jmuxer.min.js
       └── startup.sh
   ```

### 3. Automatic Installation
1. Insert the USB drive into any USB port on the Clinton DVR.
2. Reboot or power-cycle the DVR.
3. The internal firmware upgrade mechanism automatically executes `rc.upgrade.ext` as root.
4. After ~15 seconds, remove the USB drive. You will find `INSTALL_SUCCESS.txt` written to the drive.
5. The DVR is now completely upgraded!

---

## 🌐 Accessing Services

Once installed, the following services are permanently active:

* **Modern Web Console**: `http://<DVR-IP>/`
* **RTSP Video Streams**:
  * Camera 1: `rtsp://<DVR-IP>:554/live/0`
  * Camera 2: `rtsp://<DVR-IP>:554/live/1`
  * Camera 3: `rtsp://<DVR-IP>:554/live/2`
  * Camera 4: `rtsp://<DVR-IP>:554/live/3`
* **Root Telnet Shell**: `telnet <DVR-IP> 23` (port 23, no password required on LAN)

---

## 🏠 Home Assistant & Apple HomeKit Setup

No third-party custom components or HACS plugins are needed! Add directly to your Home Assistant `configuration.yaml`:

```yaml
ffmpeg:

camera:
  - platform: ffmpeg
    name: "Clinton DVR C01"
    input: "-analyzeduration 0 -probesize 32 -rtsp_transport tcp -fflags nobuffer -flags low_delay -i rtsp://10.1.1.250:554/live/0"
    extra_arguments: "-vf scale=640:480"

  - platform: ffmpeg
    name: "Clinton DVR C02"
    input: "-analyzeduration 0 -probesize 32 -rtsp_transport tcp -fflags nobuffer -flags low_delay -i rtsp://10.1.1.250:554/live/1"
    extra_arguments: "-vf scale=640:480"

homekit:
  - name: "Clinton DVR"
    port: 21064
    pincode: "201-21-013"
    filter:
      include_entities:
        - camera.clinton_dvr_c01
        - camera.clinton_dvr_c02
    entity_config:
      camera.clinton_dvr_c01:
        stream_source: "-analyzeduration 0 -probesize 32 -rtsp_transport tcp -fflags nobuffer -flags low_delay -i rtsp://10.1.1.250:554/live/0"
        video_map: "0:v:0 -vf scale=640:480"
        support_audio: false
        max_fps: 15
      camera.clinton_dvr_c02:
        stream_source: "-analyzeduration 0 -probesize 32 -rtsp_transport tcp -fflags nobuffer -flags low_delay -i rtsp://10.1.1.250:554/live/1"
        video_map: "0:v:0 -vf scale=640:480"
        support_audio: false
        max_fps: 15
```

---

## 🛠️ Building from Source

### Native Cross-Compilation (Linux)
Requires `gcc-arm-linux-gnueabi`:
```bash
sudo apt update && sudo apt install -y gcc-arm-linux-gnueabi make zip
make all
./build_and_package.sh
```

### Docker-Based Build (macOS / Windows / Linux)
```bash
docker run --rm -v $(pwd):/work -w /work debian:bookworm-slim bash -c "
    apt update && apt install -y gcc-arm-linux-gnueabi make zip
    make all
    ./build_and_package.sh
"
```

---

## 📖 Technical Documentation

* [Astel 7000 Protocol Specification & Endianness Handling](docs/PROTOCOL.md)
* [Hardware & Kernel Architecture (Faraday FA526 / Linux 2.6.15 OABI)](docs/HARDWARE.md)
* [Achieving Root Access & Exploiting rc.upgrade](docs/ROOTING.md)
* [Micro RTSP Daemon Architecture & GOP Ring Buffer](rtsp/README.md)
* [Home Assistant & HomeKit Low-Latency Tuning](homeassistant/README.md)

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).
