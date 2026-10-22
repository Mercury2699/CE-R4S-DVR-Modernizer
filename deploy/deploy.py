#!/usr/bin/env python3
"""
deploy.py - Automated deployment script for Clinton Shadow DVR (CE-R4S / LT400)
Connects via Telnet (port 23) and transfers/installs:
- /mtd/bin/astel_rtspd
- /mtd/bin/dvr_auth
- /mtd/web/index.cgi
- /mtd/web/api_status.cgi
- /mtd/web/api_exec.cgi
- /mtd/web/jmuxer.min.js
- /mtd/startup.sh
"""

import sys
import os
import socket
import time
import base64
import argparse

def telnet_cmd(s, cmd, wait_done=True, timeout=5.0):
    s.settimeout(timeout)
    token = b"___CMD_DONE___"
    full_cmd = (cmd + "\necho ___CMD_DONE___\n").encode("utf-8")
    s.sendall(full_cmd)
    
    if not wait_done:
        return ""

    out = b""
    start = time.time()
    while time.time() - start < timeout:
        try:
            chunk = s.recv(4096)
            if not chunk:
                break
            out += chunk
            if token in out:
                break
        except socket.timeout:
            break
        except Exception:
            break
    return out.decode("utf-8", errors="ignore")

def upload_file_b64(s, local_path, remote_path, mode="755"):
    print(f"[*] Uploading {os.path.basename(local_path)} -> {remote_path} (mode {mode})...")
    with open(local_path, "rb") as f:
        data = f.read()
    
    b64 = base64.b64encode(data).decode("ascii")
    tmp_path = f"/tmp/{os.path.basename(remote_path)}.tmp"

    # Use split chunks (60 chars) for standard uudecode compatibility
    lines = [b64[i:i+60] for i in range(0, len(b64), 60)]
    uue_payload = f"begin-base64 644 {tmp_path}\n" + "\n".join(lines) + "\n====\n"

    # Write payload via cat
    telnet_cmd(s, f"cat << 'EOF_UUE' | uudecode\n{uue_payload}EOF_UUE", timeout=10.0)

    # Move to destination and chmod
    telnet_cmd(s, f"mv {tmp_path} {remote_path} && chmod {mode} {remote_path}")
    print(f"[+] Successfully installed {remote_path}")

def main():
    parser = argparse.ArgumentParser(description="Deploy Clinton Shadow DVR Toolkit")
    parser.add_argument("--host", default="10.1.1.250", help="DVR IP address")
    parser.add_argument("--port", type=int, default=23, help="DVR Telnet port")
    args = parser.parse_args()

    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    print(f"=== Clinton Shadow DVR Toolkit Deployer ===")
    print(f"Target: {args.host}:{args.port}")

    s = socket.socket()
    s.settimeout(5.0)
    try:
        s.connect((args.host, args.port))
    except Exception as e:
        print(f"[-] Connection failed: {e}")
        print("    Ensure telnetd is running on the DVR.")
        sys.exit(1)

    time.sleep(0.3)
    try:
        s.recv(1024)
    except Exception:
        pass

    # Ensure target directories exist
    telnet_cmd(s, "mkdir -p /mtd/bin /mtd/web")

    # Upload files
    files = [
        (os.path.join(base_dir, "rtsp", "astel_rtspd"), "/mtd/bin/astel_rtspd", "755"),
        (os.path.join(base_dir, "web", "dvr_auth"), "/mtd/bin/dvr_auth", "755"),
        (os.path.join(base_dir, "web", "index.cgi"), "/mtd/web/index.cgi", "755"),
        (os.path.join(base_dir, "web", "api_status.cgi"), "/mtd/web/api_status.cgi", "755"),
        (os.path.join(base_dir, "web", "api_exec.cgi"), "/mtd/web/api_exec.cgi", "755"),
        (os.path.join(base_dir, "web", "jmuxer.min.js"), "/mtd/web/jmuxer.min.js", "644"),
        (os.path.join(base_dir, "deploy", "startup.sh"), "/mtd/startup.sh", "755"),
    ]

    for local, remote, mode in files:
        if os.path.exists(local):
            upload_file_b64(s, local, remote, mode)
        else:
            print(f"[!] Warning: local file {local} not found, skipping.")

    # Restart RTSP daemon
    print("[*] Restarting RTSP daemon...")
    telnet_cmd(s, "killall astel_rtspd 2>/dev/null; /mtd/bin/astel_rtspd >/tmp/astel_rtspd.log 2>&1 &")

    print("[+] Deployment completed successfully!")
    s.close()

if __name__ == "__main__":
    main()
