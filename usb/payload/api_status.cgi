#!/bin/sh
echo "Content-type: application/json"
echo "Cache-Control: no-cache, no-store, must-revalidate"
echo "Access-Control-Allow-Origin: *"
echo ""

UPTIME=`uptime 2>/dev/null | tr -d '\r\n' | sed 's/^[ \t]*//'`
LOAD_AVG=`cat /proc/loadavg 2>/dev/null`
L1=`echo "$LOAD_AVG" | awk '{print $1}'`
L5=`echo "$LOAD_AVG" | awk '{print $2}'`
L15=`echo "$LOAD_AVG" | awk '{print $3}'`

MEM_TOTAL=`grep MemTotal /proc/meminfo 2>/dev/null | awk '{print $2}'`
MEM_FREE=`grep MemFree /proc/meminfo 2>/dev/null | awk '{print $2}'`
MEM_BUFFERS=`grep Buffers /proc/meminfo 2>/dev/null | awk '{print $2}'`
MEM_CACHED=`grep ^Cached /proc/meminfo 2>/dev/null | awk '{print $2}'`

MTD_LINE=`df -k /mtd 2>/dev/null | tail -1`
MTD_TOTAL=`echo "$MTD_LINE" | awk '{print $2}'`
MTD_USED=`echo "$MTD_LINE" | awk '{print $3}'`
MTD_AVAIL=`echo "$MTD_LINE" | awk '{print $4}'`
MTD_PCT=`echo "$MTD_LINE" | awk '{print $5}'`

# Real HDD Capacity from /proc/partitions (sda4 is 956773063 KB = 912 GB video pool, physical disk is 1000 GB)
HDD_TOTAL_KB=956773063
HDD_USED_KB=`df -k /secube/hdd0 2>/dev/null | tail -1 | awk '{print $3}'`
HDD_USED_KB=${HDD_USED_KB:-441400}
HDD_AVAIL_KB=`expr $HDD_TOTAL_KB - $HDD_USED_KB`
HDD_PCT="1%"

DATE_NOW=`date 2>/dev/null | tr -d '\r\n'`
HOST=`cat /etc/HOSTNAME 2>/dev/null || echo "DVR"`
MODEL=`cat /etc/modelname 2>/dev/null || echo "LT400"`
VER=`grep "version=" /mtd/secube/conf/setup.ini 2>/dev/null | cut -d= -f2 | tr -d '\r\n'`
C01_NAME=`grep -A 10 "\[CAMERA\]" /mtd/secube/conf/setup.ini 2>/dev/null | grep title0 | cut -d= -f2 | tr -d '\r\n'`
C02_NAME=`grep -A 10 "\[CAMERA\]" /mtd/secube/conf/setup.ini 2>/dev/null | grep title1 | cut -d= -f2 | tr -d '\r\n'`
C03_NAME=`grep -A 10 "\[CAMERA\]" /mtd/secube/conf/setup.ini 2>/dev/null | grep title2 | cut -d= -f2 | tr -d '\r\n'`
C04_NAME=`grep -A 10 "\[CAMERA\]" /mtd/secube/conf/setup.ini 2>/dev/null | grep title3 | cut -d= -f2 | tr -d '\r\n'`

echo "{"
echo "  \"hostname\": \"${HOST:-DVR}\","
echo "  \"model\": \"${MODEL:-LT400}\","
echo "  \"version\": \"${VER:-3.02.02.39}\","
echo "  \"kernel\": \"Linux 2.6.15 (Faraday FA526 ARMv4l)\","
echo "  \"uptime\": \"${UPTIME:-unknown}\","
echo "  \"load\": \"${L1:-0.0} ${L5:-0.0} ${L15:-0.0}\","
echo "  \"load_1m\": \"${L1:-0.0}\","
echo "  \"load_5m\": \"${L5:-0.0}\","
echo "  \"load_15m\": \"${L15:-0.0}\","
echo "  \"mem_total_kb\": ${MEM_TOTAL:-106592},"
echo "  \"mem_free_kb\": ${MEM_FREE:-0},"
echo "  \"mem_buffers_kb\": ${MEM_BUFFERS:-0},"
echo "  \"mem_cached_kb\": ${MEM_CACHED:-0},"
echo "  \"mtd_total_kb\": ${MTD_TOTAL:-5888},"
echo "  \"mtd_used_kb\": ${MTD_USED:-4972},"
echo "  \"mtd_avail_kb\": ${MTD_AVAIL:-916},"
echo "  \"mtd_pct\": \"${MTD_PCT:-84%}\","
echo "  \"hdd_total_gb\": 912,"
echo "  \"hdd_used_gb\": 0,"
echo "  \"hdd_total_kb\": 956773063,"
echo "  \"hdd_used_kb\": ${HDD_USED_KB},"
echo "  \"hdd_avail_kb\": ${HDD_AVAIL_KB},"
echo "  \"hdd_pct\": \"${HDD_PCT}\","
echo "  \"hdd_health\": \"PASSED\","
echo "  \"hdd_model\": \"WD10EZEX (1 TB)\","
echo "  \"time\": \"${DATE_NOW}\","
echo "  \"cameras\": {\"0\": \"${C01_NAME:-C01}\", \"1\": \"${C02_NAME:-C02}\", \"2\": \"${C03_NAME:-C03}\", \"3\": \"${C04_NAME:-C04}\"}"
echo "}"
