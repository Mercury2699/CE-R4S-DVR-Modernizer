# Hardware & Kernel Architecture Notes

## 1. System Overview

* **Manufacturer**: Clinton Electronics
* **Model**: Shadow Series CE-R4S (LT400 hardware platform)
* **CPU / SoC**: Faraday Technology FA526 (ARMv4 architecture, little-endian)
  * Clock speed: ~200 MHz
  * Instruction Set: ARMv4 (32-bit ARM mode only; does not support Thumb2 or modern EABI instructions)
* **RAM**: 128 MB SDRAM
* **Internal Flash**: 16 MB SPI / NOR Flash
* **Storage**: Internal SATA / IDE HDD (Supports 1TB–2TB, partitioned into root/system, temp pool, and video database partitions)

---

## 2. Operating System & Kernel

* **Kernel**: Linux 2.6.15 (Faraday FA526-armv4l-release)
* **ABI**: **OABI** (Old ARM ABI, with legacy software interrupts `swi 0x900000 + syscall_num`)
* **C Library**: uClibc 0.9.x (embedded)

### ⚠️ Cross-Compilation Pitfalls & Workarounds
1. **Modern GLIBC Incompatibility**:
   Modern ARM toolchains target EABI (`gnueabi`) or Hard-Float (`armhf`) with Linux kernel >= 3.2. If a dynamic binary linked with modern libc is copied to the DVR, the kernel triggers `Illegal instruction` or `Kernel panic` during `ld-linux.so` relocation.
2. **Freestanding `-nostdlib` Solution**:
   To ensure 100% crash-free compatibility with Linux 2.6.15 OABI:
   * We write native C programs without standard libraries (`-nostdlib -fno-builtin`).
   * Direct inline assembly system calls (`swi 0x900000 + N`) are used for `read`, `write`, `socketcall`, `poll`, `gettimeofday`, and `exit`.
   * Binaries are linked with `-static -no-pie -march=armv4 -marm`.
   * Result: Single self-contained static binaries of only **13 KB** that run flawlessly on any vintage Linux 2.6 kernel!

---

## 3. Storage & Flash Partitioning

* `/dev/root`: Root filesystem (squashfs/cramfs read-only, mounted on `/`)
* `/dev/mtdblock3`: Writable configuration partition (JFFS2, 5.8 MB, mounted on `/mtd`)
* `/dev/sda1`: SATA disk system scratch pool (ext3, mounted on `/ide`)
* `/dev/sda3`: Video recording storage partition (proprietary indexed ring buffer, mounted on `/secube/hdd0`)
* `/tmp`: In-memory `tmpfs` RAM disk (fast I/O, ideal for buffering transient keyframes)
