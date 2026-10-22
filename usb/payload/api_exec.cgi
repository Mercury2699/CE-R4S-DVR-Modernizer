#!/bin/sh
echo "Content-type: text/plain; charset=utf-8"
echo "Cache-Control: no-cache, no-store, must-revalidate"
echo "Access-Control-Allow-Origin: *"
echo ""

ACTION=`echo "$QUERY_STRING" | sed -n 's/.*action=\([^&]*\).*/\1/p'`

# Helper to decode URL-encoded string
urldecode() {
    echo "$1" | sed 's/%2B/+/g; s/%2F/\//g; s/%3D/=/g; s/%20/ /g; s/%3A/:/g; s/%40/@/g'
}

# 1. Reboot Action
if [ "$ACTION" = "reboot" ]; then
    echo "DVR 正在安全重启中，请等待约 20 秒后重新连接..."
    sync
    sleep 1
    /sbin/reboot &
    exit 0
fi

# 2. Check Auth Action
if [ "$ACTION" = "check_auth" ]; then
    U_RAW=`echo "$QUERY_STRING" | sed -n 's/.*u=\([^&]*\).*/\1/p'`
    P_RAW=`echo "$QUERY_STRING" | sed -n 's/.*p=\([^&]*\).*/\1/p'`
    AUTH_USER=`urldecode "$U_RAW"`
    AUTH_PASS=`urldecode "$P_RAW"`
    
    if [ -x /mtd/bin/dvr_auth ] && [ -n "$AUTH_USER" ]; then
        RES=`/mtd/bin/dvr_auth "$AUTH_USER" "$AUTH_PASS" 2>/dev/null`
        if [ "$RES" = "AUTH_OK" ]; then
            echo "AUTH_OK"
            exit 0
        fi
    fi
    echo "AUTH_FAIL"
    exit 0
fi

# Decode Base64 command
decode_cmd() {
    RUN_B64=`echo "$QUERY_STRING" | sed -n 's/.*run=\([^&]*\).*/\1/p'`
    RUN_CLEAN=`echo "$RUN_B64" | sed 's/%2B/+/g; s/%2F/\//g; s/%3D/=/g'`
    rm -f /tmp/web_cmd
    (echo "begin-base64 644 /tmp/web_cmd"; echo "$RUN_CLEAN"; echo "====") | uudecode 2>/dev/null
    CMD_STR=`cat /tmp/web_cmd 2>/dev/null`
    rm -f /tmp/web_cmd
    if [ -z "$CMD_STR" ]; then
        CMD_STR=`echo "$QUERY_STRING" | sed -n 's/.*cmd_raw=\([^&]*\).*/\1/p' | sed 's/%20/ /g'`
    fi
}

# 3. Preset Commands (No Authentication Required)
if [ "$ACTION" = "preset" ]; then
    decode_cmd
    if [ -z "$CMD_STR" ]; then
        echo "Error: Empty preset command."
        exit 0
    fi

    # Whitelist check for safety
    case "$CMD_STR" in
        "uname -a"|\
        "uptime"|\
        "free"|\
        "df -h"|\
        "cat /proc/cpuinfo"|\
        "ps"|\
        "ps | grep -E 'nova|astel|webfsd'"|\
        'ps | grep -E "nova|astel|webfsd"'|\
        "netstat -tlpn"|\
        "top -n 1"|\
        "ifconfig"|\
        "route -n"|\
        "arp -a"|\
        "ping -c 3 10.1.1.1"|\
        "ls -la /mtd"|\
        "ls -lh /mtd/bin"|\
        "ls -la /ide"|\
        "dmesg | tail -n 25")
            eval "$CMD_STR" 2>&1 | head -n 80
            exit 0
            ;;
        *)
            echo "Error: Command not in authorized preset list."
            exit 0
            ;;
    esac
fi

# 4. Custom Shell Execution (Requires Authentication)
if [ "$ACTION" = "sh" ]; then
    U_RAW=`echo "$QUERY_STRING" | sed -n 's/.*u=\([^&]*\).*/\1/p'`
    P_RAW=`echo "$QUERY_STRING" | sed -n 's/.*p=\([^&]*\).*/\1/p'`
    AUTH_USER=`urldecode "$U_RAW"`
    AUTH_PASS=`urldecode "$P_RAW"`

    # Check authentication against DVR config
    AUTH_CHECK=""
    if [ -x /mtd/bin/dvr_auth ] && [ -n "$AUTH_USER" ]; then
        AUTH_CHECK=`/mtd/bin/dvr_auth "$AUTH_USER" "$AUTH_PASS" 2>/dev/null`
    fi

    if [ "$AUTH_CHECK" != "AUTH_OK" ]; then
        echo "🔒 认证失败：执行自定义 Shell 命令需要通过管理员身份验证。"
        echo "请提供正确的管理员用户名及密码。"
        exit 0
    fi

    decode_cmd
    if [ -n "$CMD_STR" ]; then
        eval "$CMD_STR" 2>&1 | head -n 80
    else
        echo "Error: Empty command received."
    fi
    exit 0
fi

echo "Error: Unknown action"
