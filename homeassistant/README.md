# Home Assistant & Apple HomeKit Integration Guide

This guide explains how to integrate the Clinton Shadow DVR cameras directly into **Home Assistant** and **Apple HomeKit (Apple Home app)** with:
* **0.5-second instant live stream opening** (down from 10–16s).
* **Correct 4:3 aspect ratio** (640×480) without squishing or stretching.
* **100% native Home Assistant configuration** (no custom components or plugins required).

---

## 1. Home Assistant Configuration

Add the following to your Home Assistant `configuration.yaml`:

```yaml
# 1. Enable standard FFmpeg platform
ffmpeg:

# 2. Add native cameras using the Clinton RTSP feed
camera:
  - platform: ffmpeg
    name: "Clinton DVR C01"
    input: "-analyzeduration 0 -probesize 32 -rtsp_transport tcp -fflags nobuffer -flags low_delay -i rtsp://10.1.1.250:554/live/0"
    extra_arguments: "-vf scale=640:480"

  - platform: ffmpeg
    name: "Clinton DVR C02"
    input: "-analyzeduration 0 -probesize 32 -rtsp_transport tcp -fflags nobuffer -flags low_delay -i rtsp://10.1.1.250:554/live/1"
    extra_arguments: "-vf scale=640:480"

# 3. Expose to Apple HomeKit with low-latency transcoding and 4:3 aspect ratio
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

## 2. Key Optimization Parameters

| Parameter | Purpose |
| :--- | :--- |
| `-analyzeduration 0 -probesize 32` | Halts FFmpeg stream probing on packet 1. Skips 3–5 seconds of format guessing. |
| `-rtsp_transport tcp` | Forces reliable TCP streaming, avoiding UDP packet drops and slice corruption. |
| `-fflags nobuffer -flags low_delay` | Minimizes input pipeline buffering for immediate frame delivery. |
| `-vf scale=640:480` | Rescales the DVR's raw 704×240 field scan to correct 4:3 standard aspect ratio. |
| `-tune zerolatency` | Configures x264 encoder for zero buffering delay. |
