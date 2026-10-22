# Clinton Electronics Shadow DVR (Astel / Nova) Protocol Specification

## 1. Overview
Clinton Electronics Shadow Series DVRs (CE-R4S, LT400, and related series) communicate with client applications (such as CVMS and the legacy CEC Connect iOS/macOS app) over TCP port **7000** (Server Management Software / SMS port).

The streaming subsystem is managed by the internal `nova` daemon (`/secube/bin/nova_dvr`).

---

## 2. Framing and Header Structure

All packets sent and received on port 7000 share a common 12-byte header:

```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                 Payload Length (Big-Endian, 4 bytes)          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Reserved     |  Magic (0xA1) |  Opcode (1B)  | Subcode (1B)  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Seq / Status |          Size (Little-Endian, 3 bytes)        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

* **Bytes 0–3**: Total Packet Size minus 4 (Big-Endian `uint32_t`).
* **Byte 4**: `0x00` (Header delimiter).
* **Byte 5**: Magic identifier `0xA1`.
* **Byte 6**: Command Opcode:
  * `0x01`: Authentication Request (Login)
  * `0x02`: Authentication Response
  * `0x03`: Video Streaming Request
  * `0x29`: Incoming Live Video Data Frame
  * `0x05`: Video Streaming Stop / Teardown
  * `0x07`: Keep-Alive / Heartbeat
* **Byte 7**: Status code (`0x00` = Success, `0x01` = Authentication Failed).
* **Bytes 9–11**: Total Packet Size (Little-Endian `uint24`).

---

## 3. Handshake & Stream Request

### 3.1 Authentication (Opcode 0x01)
Client sends:
```
Offset  Size  Description
12      1     Padding (0x00)
13      1     Username length (ulen)
14      1     Password length (plen)
15      ulen  ASCII Username (e.g., "wang" or "r00t")
15+ulen plen  ASCII Password
```

DVR responds with Opcode `0x02`. If byte 7 is `0x00`, authentication is successful.

### 3.2 Video Streaming Request (Opcode 0x03)
Client requests specific channels and framerate:
```
Offset  Size  Description
12      2     Channel bitmask (Little-Endian uint16):
              - 0x01: Channel 0 (Camera 1)
              - 0x02: Channel 1 (Camera 2)
              - 0x04: Channel 2 (Camera 3)
              - 0x08: Channel 3 (Camera 4)
              - 0x0F: All 4 Channels
14      1     Requested Framerate (e.g., 30 fps)
15      1     Resolution mode (0 = CIF/Field, 1 = D1)
```

---

## 4. Video Data Framing (Opcode 0x29) & The 32-Bit Endianness Quirk

When Nova streams live video packets (Opcode `0x29`), the payload contains an internal stream header followed by raw H.264 compressed frame data:

```
Offset  Size  Description
12      1     Stream Flag byte (if bit 0x40 or 0x80 is set, header is 40 bytes; otherwise 32 bytes)
14      1     Camera Channel Index (0, 1, 2, or 3)
20      4     Frame Timestamp (Hardware ticks)
Header_Len    H.264 Compressed Data
```

### ⚠️ The 32-Bit Endianness Byte Swap
The raw hardware encoder engine inside the Faraday FA526 SoC writes 32-bit words into memory in reverse word endianness relative to standard byte streams. 

If ingested directly by standard decoders (FFmpeg, browsers), the stream will report corrupted headers (`Invalid NAL unit size`, `non-existing PPS 0 referenced`).

**The Solution:**
Every 4-byte chunk of the payload starting from the NAL unit boundary must be byte-reversed in 32-bit increments:

```c
/* In-place 32-bit word swap for standard H.264 byte stream recovery */
uint32_t *p = (uint32_t *)nal_data;
int words = nal_len / 4;
for (int i = 0; i < words; i++) {
    uint32_t v = p[i];
    p[i] = ((v & 0xFF) << 24) |
           ((v & 0xFF00) << 8) |
           ((v >> 8) & 0xFF00) |
           ((v >> 24) & 0xFF);
}
```

Once unswapped, the stream conforms strictly to standard ISO/IEC 14496-10 (H.264 Baseline Profile) with Annex B start codes (`00 00 00 01`).
