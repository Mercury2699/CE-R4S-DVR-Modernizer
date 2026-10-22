#!/usr/bin/env bash
#
# build_and_package.sh - One-Click Build & USB Package Generator
# Clinton Electronics Shadow Series DVR Toolkit
#

set -e

REPO_DIR="$(cd "$(dirname "$0")" && pwd)"
DIST_DIR="$REPO_DIR/dist"
USB_DIR="$DIST_DIR/usb_root"
PAYLOAD_DIR="$USB_DIR/payload"

echo "=========================================================="
echo " Clinton Electronics Shadow DVR Toolkit - Build & Packager"
echo "=========================================================="

mkdir -p "$DIST_DIR" "$USB_DIR" "$PAYLOAD_DIR"

# 1. Compilation
BUILD_SUCCESS=0
if command -v arm-linux-gnueabi-gcc >/dev/null 2>&1; then
    echo "[1/4] Found local ARM cross-compiler (arm-linux-gnueabi-gcc). Building..."
    make -C "$REPO_DIR" clean all
    cp -f "$REPO_DIR/rtsp/astel_rtspd" "$REPO_DIR/bin/prebuilt/astel_rtspd"
    cp -f "$REPO_DIR/web/dvr_auth" "$REPO_DIR/bin/prebuilt/dvr_auth"
    BUILD_SUCCESS=1
elif command -v docker >/dev/null 2>&1 && docker info >/dev/null 2>&1; then
    echo "[1/4] Using Docker cross-compilation environment (Debian arm-linux-gnueabi)..."
    docker run --rm -v "$REPO_DIR:/work" -w /work debian:bookworm-slim bash -c "
        apt-get update -qq && apt-get install -qq -y gcc-arm-linux-gnueabi make >/dev/null
        make clean all
        cp -f rtsp/astel_rtspd bin/prebuilt/astel_rtspd
        cp -f web/dvr_auth bin/prebuilt/dvr_auth
    "
    BUILD_SUCCESS=1
else
    echo "[1/4] No local ARM toolchain or Docker found. Using verified prebuilt ARMv4 binaries..."
fi

# Verify binaries exist
if [ ! -f "$REPO_DIR/bin/prebuilt/astel_rtspd" ] || [ ! -f "$REPO_DIR/bin/prebuilt/dvr_auth" ]; then
    echo "[-] Error: Prebuilt binaries not found in bin/prebuilt!"
    exit 1
fi

echo "[2/4] Assembling USB installer payload..."
cp -f "$REPO_DIR/bin/prebuilt/astel_rtspd" "$PAYLOAD_DIR/astel_rtspd"
cp -f "$REPO_DIR/bin/prebuilt/dvr_auth" "$PAYLOAD_DIR/dvr_auth"
cp -f "$REPO_DIR/web/index.cgi" "$PAYLOAD_DIR/index.cgi"
cp -f "$REPO_DIR/web/api_status.cgi" "$PAYLOAD_DIR/api_status.cgi"
cp -f "$REPO_DIR/web/api_exec.cgi" "$PAYLOAD_DIR/api_exec.cgi"
cp -f "$REPO_DIR/web/jmuxer.min.js" "$PAYLOAD_DIR/jmuxer.min.js"
cp -f "$REPO_DIR/deploy/startup.sh" "$PAYLOAD_DIR/startup.sh"

echo "[3/4] Copying USB auto-install trigger scripts..."
cp -f "$REPO_DIR/usb/rc.upgrade.ext" "$USB_DIR/rc.upgrade.ext"
cp -f "$REPO_DIR/usb/install.sh" "$USB_DIR/install.sh"
chmod +x "$USB_DIR/rc.upgrade.ext" "$USB_DIR/install.sh"
chmod +x "$PAYLOAD_DIR/astel_rtspd" "$PAYLOAD_DIR/dvr_auth" "$PAYLOAD_DIR/"*.cgi

echo "[4/4] Creating USB distribution zip archive..."
ZIP_NAME="CE-R4S-Modernizer-v1.0.0-USB.zip"
ZIP_PATH="$DIST_DIR/$ZIP_NAME"
rm -f "$ZIP_PATH"

(cd "$USB_DIR" && zip -r "$ZIP_PATH" rc.upgrade.ext install.sh payload/)

echo ""
echo "=========================================================="
echo " Build & Packaging Complete!"
echo " Output archive: $ZIP_PATH"
echo " Size: $(ls -lh "$ZIP_PATH" | awk '{print $5}')"
echo "=========================================================="
echo ""
echo "USB Flash Drive Installation Guide:"
echo "1. Format a USB drive as FAT32 (or MBR/FAT)."
echo "2. Unzip '$ZIP_NAME' directly to the ROOT directory of the USB drive:"
echo "   /Volumes/USB/"
echo "   ├── rc.upgrade.ext"
echo "   ├── install.sh"
echo "   └── payload/"
echo "       ├── astel_rtspd"
echo "       ├── dvr_auth"
echo "       ├── index.cgi"
echo "       ├── api_status.cgi"
echo "       ├── api_exec.cgi"
echo "       ├── jmuxer.min.js"
echo "       └── startup.sh"
echo "3. Insert the USB drive into ANY USB port on the Clinton DVR."
echo "4. Power-on or restart the DVR. The firmware auto-upgrade handler"
echo "   (/etc/rc.d/rc.upgrade) will execute rc.upgrade.ext automatically."
echo "5. Wait ~15 seconds until 'INSTALL_SUCCESS.txt' is written to the USB drive."
echo "6. Access the modern Web UI at http://<DVR-IP>/ and RTSP at rtsp://<DVR-IP>:554/live/0 !"
echo "=========================================================="
