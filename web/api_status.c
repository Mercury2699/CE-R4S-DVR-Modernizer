/* api_status.c - Freestanding C CGI for Clinton DVR status */
#define NULL ((void*)0)

#define __NR_SYSCALL_BASE 0x900000
#define __NR_exit         (__NR_SYSCALL_BASE + 1)
#define __NR_read         (__NR_SYSCALL_BASE + 3)
#define __NR_write        (__NR_SYSCALL_BASE + 4)
#define __NR_open         (__NR_SYSCALL_BASE + 5)
#define __NR_close        (__NR_SYSCALL_BASE + 6)

#define O_RDONLY 0

static inline long sys_write(int fd, const void *buf, unsigned long count) {
    register long r0 __asm__("r0") = fd;
    register long r1 __asm__("r1") = (long)buf;
    register long r2 __asm__("r2") = count;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_write), "0"(r0), "r"(r1), "r"(r2) : "memory");
    return r0;
}

static inline long sys_read(int fd, void *buf, unsigned long count) {
    register long r0 __asm__("r0") = fd;
    register long r1 __asm__("r1") = (long)buf;
    register long r2 __asm__("r2") = count;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_read), "0"(r0), "r"(r1), "r"(r2) : "memory");
    return r0;
}

static inline int sys_open(const char *pathname, int flags) {
    register long r0 __asm__("r0") = (long)pathname;
    register long r1 __asm__("r1") = flags;
    register long r2 __asm__("r2") = 0;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_open), "0"(r0), "r"(r1), "r"(r2) : "memory");
    return r0;
}

static inline int sys_close(int fd) {
    register long r0 __asm__("r0") = fd;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_close), "0"(r0) : "memory");
    return r0;
}

static inline void sys_exit(int status) {
    register long r0 __asm__("r0") = status;
    __asm__ __volatile__("swi %1\n" : : "r"(r0), "i"(__NR_exit) : "memory");
    while(1);
}

static unsigned long u_div(unsigned long num, unsigned long den) {
    if (den == 0) return 0;
    unsigned long quot = 0, q = 1;
    while ((den <= num) && !(den & (1UL << 31))) {
        den <<= 1;
        q <<= 1;
    }
    while (q > 0) {
        if (num >= den) {
            num -= den;
            quot += q;
        }
        den >>= 1;
        q >>= 1;
    }
    return quot;
}

static unsigned long u_mod(unsigned long num, unsigned long den) {
    if (den == 0) return 0;
    unsigned long d = den;
    while ((d <= (num >> 1)) && !(d & (1UL << 31))) {
        d <<= 1;
    }
    while (d >= den) {
        if (num >= d) num -= d;
        d >>= 1;
    }
    return num;
}

static void print_str(const char *s) {
    const char *p = s;
    while (*p) p++;
    sys_write(1, s, p - s);
}

static char* find_mem_val(const char *buf, const char *key) {
    const char *p = buf;
    const char *k = key;
    int klen = 0;
    while (k[klen]) klen++;

    while (*p) {
        int match = 1;
        for (int i = 0; i < klen; i++) {
            if (p[i] != key[i]) { match = 0; break; }
        }
        if (match) {
            p += klen;
            while (*p == ' ' || *p == '\t' || *p == ':') p++;
            return (char*)p;
        }
        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
    }
    return NULL;
}

void _start(void) {
    char lbuf[128];
    char mbuf[1024];
    char ubuf[64];

    // Read /proc/loadavg
    int l1_s = 0, l1_len = 0, l5_s = 0, l5_len = 0, l15_s = 0, l15_len = 0;
    int fd = sys_open("/proc/loadavg", O_RDONLY);
    if (fd >= 0) {
        int n = sys_read(fd, lbuf, sizeof(lbuf) - 1);
        sys_close(fd);
        if (n > 0) {
            lbuf[n] = '\0';
            int idx = 0;
            while (lbuf[idx] == ' ') idx++;
            l1_s = idx;
            while (lbuf[idx] && lbuf[idx] != ' ') idx++;
            l1_len = idx - l1_s;

            while (lbuf[idx] == ' ') idx++;
            l5_s = idx;
            while (lbuf[idx] && lbuf[idx] != ' ') idx++;
            l5_len = idx - l5_s;

            while (lbuf[idx] == ' ') idx++;
            l15_s = idx;
            while (lbuf[idx] && lbuf[idx] != ' ') idx++;
            l15_len = idx - l15_s;
        }
    }

    // Read /proc/uptime
    unsigned long uptime_sec = 0;
    fd = sys_open("/proc/uptime", O_RDONLY);
    if (fd >= 0) {
        int n = sys_read(fd, ubuf, sizeof(ubuf) - 1);
        sys_close(fd);
        if (n > 0) {
            ubuf[n] = '\0';
            for (int i = 0; i < n && ubuf[i] >= '0' && ubuf[i] <= '9'; i++) {
                uptime_sec = uptime_sec * 10 + (ubuf[i] - '0');
            }
        }
    }

    // Read /proc/meminfo
    char *p_total = NULL, *p_free = NULL, *p_buf = NULL, *p_cache = NULL;
    fd = sys_open("/proc/meminfo", O_RDONLY);
    if (fd >= 0) {
        int n = sys_read(fd, mbuf, sizeof(mbuf) - 1);
        sys_close(fd);
        if (n > 0) {
            mbuf[n] = '\0';
            p_total = find_mem_val(mbuf, "MemTotal");
            p_free = find_mem_val(mbuf, "MemFree");
            p_buf = find_mem_val(mbuf, "Buffers");
            p_cache = find_mem_val(mbuf, "Cached");
        }
    }

    // Print CGI HTTP headers
    print_str("Content-type: application/json\r\nCache-Control: no-cache, no-store, must-revalidate\r\nAccess-Control-Allow-Origin: *\r\n\r\n");

    // Output JSON
    print_str("{\n  \"hostname\": \"DVR\",\n  \"model\": \"LT400\",\n  \"version\": \"3.02.02.39\",\n  \"kernel\": \"Linux 2.6.15 (Faraday FA526 ARMv4l)\",\n");

    // Uptime formatting
    unsigned long days = u_div(uptime_sec, 86400);
    unsigned long day_rem = u_mod(uptime_sec, 86400);
    unsigned long hours = u_div(day_rem, 3600);
    unsigned long hour_rem = u_mod(day_rem, 3600);
    unsigned long mins = u_div(hour_rem, 60);

    print_str("  \"uptime\": \"up ");
    if (days > 0) {
        char dstr[16]; int didx = 0; unsigned long dtmp = days;
        while (dtmp > 0) {
            dstr[didx++] = '0' + u_mod(dtmp, 10);
            dtmp = u_div(dtmp, 10);
        }
        for (int i = didx - 1; i >= 0; i--) sys_write(1, &dstr[i], 1);
        print_str(" days, ");
    }
    char numbuf[8];
    numbuf[0] = '0' + u_div(hours, 10);
    numbuf[1] = '0' + u_mod(hours, 10);
    numbuf[2] = ':';
    numbuf[3] = '0' + u_div(mins, 10);
    numbuf[4] = '0' + u_mod(mins, 10);
    sys_write(1, numbuf, 5);
    print_str("\",\n");

    // Load averages
    print_str("  \"load_1m\": \"");
    if (l1_len > 0) sys_write(1, lbuf + l1_s, l1_len); else print_str("0.00");
    print_str("\",\n  \"load_5m\": \"");
    if (l5_len > 0) sys_write(1, lbuf + l5_s, l5_len); else print_str("0.00");
    print_str("\",\n  \"load_15m\": \"");
    if (l15_len > 0) sys_write(1, lbuf + l15_s, l15_len); else print_str("0.00");
    print_str("\",\n");

    // Mem info
    print_str("  \"mem_total_kb\": ");
    if (p_total) { while (*p_total >= '0' && *p_total <= '9') { sys_write(1, p_total++, 1); } } else print_str("106592");
    print_str(",\n  \"mem_free_kb\": ");
    if (p_free) { while (*p_free >= '0' && *p_free <= '9') { sys_write(1, p_free++, 1); } } else print_str("0");
    print_str(",\n  \"mem_buffers_kb\": ");
    if (p_buf) { while (*p_buf >= '0' && *p_buf <= '9') { sys_write(1, p_buf++, 1); } } else print_str("0");
    print_str(",\n  \"mem_cached_kb\": ");
    if (p_cache) { while (*p_cache >= '0' && *p_cache <= '9') { sys_write(1, p_cache++, 1); } } else print_str("0");
    print_str(",\n");

    // Static MTD and HDD info
    print_str("  \"mtd_total_kb\": 5888,\n  \"mtd_used_kb\": 4972,\n  \"mtd_avail_kb\": 916,\n  \"mtd_pct\": \"84%\",\n");
    print_str("  \"hdd_total_gb\": 912,\n  \"hdd_used_gb\": 0,\n  \"hdd_total_kb\": 956773063,\n  \"hdd_used_kb\": 441400,\n  \"hdd_avail_kb\": 956331663,\n  \"hdd_pct\": \"1%\",\n");
    print_str("  \"hdd_health\": \"PASSED\",\n  \"hdd_model\": \"WD10EZEX (1 TB)\",\n");
    print_str("  \"cameras\": {\"0\": \"Front Door\", \"1\": \"Side Yard\", \"2\": \"C03\", \"3\": \"C04\"}\n}\n");

    sys_exit(0);
}
