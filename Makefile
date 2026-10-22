# Makefile for Clinton Shadow DVR Toolkit (ARMv4 / FA526 Linux 2.6.15)
# Cross-compiler: arm-linux-gnueabi-gcc

CC = arm-linux-gnueabi-gcc
STRIP = arm-linux-gnueabi-strip

# Freestanding ARMv4 flags (OABI syscalls, zero libc dependency)
CFLAGS_FREESTANDING = -march=armv4 -marm -nostdlib -fno-builtin -no-pie -static -O2 -Wall
# Standard static flags
CFLAGS_STATIC = -march=armv4 -marm -static -no-pie -O2 -Wall

all: rtsp/astel_rtspd web/dvr_auth web/api_status

rtsp/astel_rtspd: rtsp/astel_rtspd.c
	@echo "[CC] Building astel_rtspd (freestanding ARMv4 RTSP server)..."
	$(CC) $(CFLAGS_FREESTANDING) $< -o $@
	$(STRIP) -s $@

web/dvr_auth: web/dvr_auth.c
	@echo "[CC] Building dvr_auth (freestanding ARMv4 credential verifier)..."
	$(CC) $(CFLAGS_FREESTANDING) $< -o $@
	$(STRIP) -s $@

web/api_status: web/api_status.c
	@echo "[CC] Building api_status (telemetry binary)..."
	$(CC) $(CFLAGS_FREESTANDING) $< -o $@
	$(STRIP) -s $@

clean:
	rm -f rtsp/astel_rtspd web/dvr_auth web/api_status

.PHONY: all clean
