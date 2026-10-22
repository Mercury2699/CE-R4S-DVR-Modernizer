/*
 * astel_rtspd.c - Native standalone micro RTSP daemon for ASTEL-LT (FA526 / ARMv4l)
 * Reverse engineered for Clinton Electronics CE-R4S DVR running Linux 2.6.15 (OABI).
 * Zero external library dependencies (freestanding -nostdlib ARMv4 binary).
 *
 * Connects to local nova daemon at 127.0.0.1:7000, performs in-memory 32-bit word unswapping,
 * encapsulates H.264 NALs into RFC 6184 RTP packets, and serves standard RTSP on port 554.
 */

#define NULL ((void*)0)

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef int            int32_t;
typedef long           intptr_t;
typedef unsigned long  uintptr_t;

/* --- ARM Linux OABI System Calls --- */
#define __NR_SYSCALL_BASE 0x900000
#define __NR_exit         (__NR_SYSCALL_BASE + 1)
#define __NR_fork         (__NR_SYSCALL_BASE + 2)
#define __NR_read         (__NR_SYSCALL_BASE + 3)
#define __NR_write        (__NR_SYSCALL_BASE + 4)
#define __NR_open         (__NR_SYSCALL_BASE + 5)
#define __NR_close        (__NR_SYSCALL_BASE + 6)
#define __NR_signal       (__NR_SYSCALL_BASE + 48)
#define __NR_gettimeofday (__NR_SYSCALL_BASE + 78)
#define __NR_socketcall   (__NR_SYSCALL_BASE + 102)
#define __NR_poll         (__NR_SYSCALL_BASE + 168)
#define __NR_fcntl        (__NR_SYSCALL_BASE + 55)

#define SYS_SOCKET      1
#define SYS_BIND        2
#define SYS_CONNECT     3
#define SYS_LISTEN      4
#define SYS_ACCEPT      5
#define SYS_GETSOCKNAME 6
#define SYS_GETPEERNAME 7
#define SYS_SEND        9
#define SYS_RECV        10
#define SYS_SENDTO      11
#define SYS_RECVFROM    12
#define SYS_SETSOCKOPT  14

#define AF_INET     2
#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOL_SOCKET  1
#define SO_REUSEADDR 2
#define F_GETFL     3
#define F_SETFL     4
#define O_NONBLOCK  04000
#define O_WRONLY    01
#define O_CREAT     0100
#define O_TRUNC     01000

#define POLLIN  0x0001
#define POLLPRI 0x0002
#define POLLOUT 0x0004
#define POLLERR 0x0008
#define POLLHUP 0x0010
#define MSG_NOSIGNAL 0x4000

struct pollfd {
    int   fd;
    short events;
    short revents;
};

struct timeval {
    long tv_sec;
    long tv_usec;
};

struct in_addr {
    uint32_t s_addr;
};

struct sockaddr_in {
    uint16_t sin_family;
    uint16_t sin_port;
    struct in_addr sin_addr;
    uint8_t  sin_zero[8];
};

static inline void sys_exit(int code) {
    register long r0 __asm__("r0") = code;
    __asm__ __volatile__("swi %1\n" : : "r"(r0), "i"(__NR_exit) : "memory");
    while(1);
}

static inline long sys_read(int fd, void *buf, unsigned long count) {
    register long r0 __asm__("r0") = fd;
    register long r1 __asm__("r1") = (long)buf;
    register long r2 __asm__("r2") = count;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_read), "0"(r0), "r"(r1), "r"(r2) : "memory");
    return r0;
}

static inline long sys_write(int fd, const void *buf, unsigned long count) {
    register long r0 __asm__("r0") = fd;
    register long r1 __asm__("r1") = (long)buf;
    register long r2 __asm__("r2") = count;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_write), "0"(r0), "r"(r1), "r"(r2) : "memory");
    return r0;
}

static inline long sys_open(const char *path, int flags, int mode) {
    register long r0 __asm__("r0") = (long)path;
    register long r1 __asm__("r1") = flags;
    register long r2 __asm__("r2") = mode;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_open), "0"(r0), "r"(r1), "r"(r2) : "memory");
    return r0;
}

static inline long sys_close(int fd) {
    register long r0 __asm__("r0") = fd;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_close), "0"(r0) : "memory");
    return r0;
}

static inline long sys_signal(int sig, unsigned long handler) {
    register long r0 __asm__("r0") = sig;
    register long r1 __asm__("r1") = handler;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_signal), "0"(r0), "r"(r1) : "memory");
    return r0;
}

static inline long sys_gettimeofday(struct timeval *tv, void *tz) {
    register long r0 __asm__("r0") = (long)tv;
    register long r1 __asm__("r1") = (long)tz;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_gettimeofday), "0"(r0), "r"(r1) : "memory");
    return r0;
}

static inline long sys_poll(struct pollfd *fds, unsigned int nfds, int timeout) {
    register long r0 __asm__("r0") = (long)fds;
    register long r1 __asm__("r1") = nfds;
    register long r2 __asm__("r2") = timeout;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_poll), "0"(r0), "r"(r1), "r"(r2) : "memory");
    return r0;
}

static inline long sys_fcntl(int fd, int cmd, long arg) {
    register long r0 __asm__("r0") = fd;
    register long r1 __asm__("r1") = cmd;
    register long r2 __asm__("r2") = arg;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_fcntl), "0"(r0), "r"(r1), "r"(r2) : "memory");
    return r0;
}

static inline long sys_socketcall(int call, unsigned long *args) {
    register long r0 __asm__("r0") = call;
    register long r1 __asm__("r1") = (long)args;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_socketcall), "0"(r0), "r"(r1) : "memory");
    return r0;
}

/* Socket helpers */
static int sock_socket(int domain, int type, int protocol) {
    unsigned long a[3] = { domain, type, protocol };
    return sys_socketcall(SYS_SOCKET, a);
}

static int sock_connect(int fd, struct sockaddr_in *addr, int addrlen) {
    unsigned long a[3] = { fd, (unsigned long)addr, addrlen };
    return sys_socketcall(SYS_CONNECT, a);
}

static int sock_bind(int fd, struct sockaddr_in *addr, int addrlen) {
    unsigned long a[3] = { fd, (unsigned long)addr, addrlen };
    return sys_socketcall(SYS_BIND, a);
}

static int sock_listen(int fd, int backlog) {
    unsigned long a[2] = { fd, backlog };
    return sys_socketcall(SYS_LISTEN, a);
}

static int sock_accept(int fd, struct sockaddr_in *addr, int *addrlen) {
    unsigned long a[3] = { fd, (unsigned long)addr, (unsigned long)addrlen };
    return sys_socketcall(SYS_ACCEPT, a);
}

static int sock_setsockopt(int fd, int level, int optname, const void *optval, int optlen) {
    unsigned long a[5] = { fd, level, optname, (unsigned long)optval, optlen };
    return sys_socketcall(SYS_SETSOCKOPT, a);
}

static int sock_send(int fd, const void *buf, int len, int flags) {
    unsigned long a[4] = { fd, (unsigned long)buf, len, flags | MSG_NOSIGNAL };
    return sys_socketcall(SYS_SEND, a);
}

static int sock_recv(int fd, void *buf, int len, int flags) {
    unsigned long a[4] = { fd, (unsigned long)buf, len, flags };
    return sys_socketcall(SYS_RECV, a);
}

static int sock_sendto(int fd, const void *buf, int len, int flags, struct sockaddr_in *addr, int addrlen) {
    unsigned long a[6] = { fd, (unsigned long)buf, len, flags | MSG_NOSIGNAL, (unsigned long)addr, addrlen };
    return sys_socketcall(SYS_SENDTO, a);
}

static int send_all(int fd, const void *buf, int len) {
    const uint8_t *p = (const uint8_t *)buf;
    int remaining = len;
    while (remaining > 0) {
        int n = sock_send(fd, p, remaining, 0);
        if (n < 0) {
            if (n == -11 || n == -4) {
                struct pollfd pfd;
                pfd.fd = fd;
                pfd.events = POLLOUT;
                pfd.revents = 0;
                int pr = sys_poll(&pfd, 1, 100);
                if (pr <= 0) return -1;
                continue;
            }
            return -1;
        }
        if (n == 0) return -1;
        p += n;
        remaining -= n;
    }
    return 0;
}

static void set_nonblock(int fd) {
    long flags = sys_fcntl(fd, F_GETFL, 0);
    if (flags >= 0) {
        sys_fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
}

/* --- Basic String / Memory Utilities --- */
void *memcpy(void *dest, const void *src, unsigned long n) {
    char *d = (char*)dest;
    const char *s = (const char*)src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memset(void *s, int c, unsigned long n) {
    unsigned char *p = (unsigned char*)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void *memmove(void *dest, const void *src, unsigned long n) {
    char *d = (char*)dest;
    const char *s = (const char*)src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

int m_strlen(const char *s) {
    int len = 0;
    while (*s++) len++;
    return len;
}

unsigned long strlen(const char *s) {
    unsigned long len = 0;
    while (*s++) len++;
    return len;
}

static void *m_memcpy(void *dest, const void *src, unsigned long n) {
    return memcpy(dest, src, n);
}

static void *m_memset(void *s, int c, unsigned long n) {
    return memset(s, c, n);
}

static int m_strncmp(const char *s1, const char *s2, unsigned long n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++; s2++; n--;
    }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static const char *m_strstr(const char *haystack, const char *needle) {
    int nlen = m_strlen(needle);
    if (nlen == 0) return haystack;
    while (*haystack) {
        if (*haystack == *needle && m_strncmp(haystack, needle, nlen) == 0) {
            return haystack;
        }
        haystack++;
    }
    return NULL;
}

static void print_str(const char *str) {
    sys_write(1, str, m_strlen(str));
}

static void print_int(long val) {
    char buf[32];
    int idx = 0;
    int is_neg = 0;
    if (val < 0) {
        is_neg = 1;
        val = -val;
    }
    if (val == 0) {
        buf[idx++] = '0';
    } else {
        while (val > 0) {
            buf[idx++] = '0' + (val % 10);
            val /= 10;
        }
    }
    if (is_neg) buf[idx++] = '-';
    // reverse
    for (int i = 0; i < idx / 2; i++) {
        char tmp = buf[i];
        buf[i] = buf[idx - 1 - i];
        buf[idx - 1 - i] = tmp;
    }
    sys_write(1, buf, idx);
}

static uint16_t htons(uint16_t v) {
    return (v << 8) | (v >> 8);
}

static uint32_t htonl(uint32_t v) {
    return ((v & 0xff) << 24) | ((v & 0xff00) << 8) | ((v & 0xff0000) >> 8) | ((v >> 24) & 0xff);
}

static const char b64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int base64_encode(const uint8_t *in, int in_len, char *out, int out_max) {
    int o = 0;
    for (int i = 0; i < in_len; i += 3) {
        if (o + 4 >= out_max) break;
        uint32_t b0 = in[i];
        uint32_t b1 = (i + 1 < in_len) ? in[i + 1] : 0;
        uint32_t b2 = (i + 2 < in_len) ? in[i + 2] : 0;
        uint32_t triple = (b0 << 16) | (b1 << 8) | b2;

        out[o++] = b64_chars[(triple >> 18) & 0x3f];
        out[o++] = b64_chars[(triple >> 12) & 0x3f];
        out[o++] = (i + 1 < in_len) ? b64_chars[(triple >> 6) & 0x3f] : '=';
        out[o++] = (i + 2 < in_len) ? b64_chars[triple & 0x3f] : '=';
    }
    out[o] = '\0';
    return o;
}

/* --- Constants & Buffers --- */
#define NOVA_PORT 7000
#define RTSP_PORT 554
#define MAX_CLIENTS 8
#define MAX_CHANNELS 4

#define STATE_FREE     0
#define STATE_INIT     1
#define STATE_READY    2
#define STATE_PLAYING  3

#define TRANSPORT_TCP 1
#define TRANSPORT_UDP 2

struct rtsp_client {
    int state;
    int fd;
    int transport_type; // TRANSPORT_TCP or TRANSPORT_UDP
    int channel;        // 0..3
    int rtp_ch;         // interleaved TCP channel (usually 0)
    int rtcp_ch;        // interleaved TCP channel (usually 1)
    
    int udp_rtp_fd;
    struct sockaddr_in udp_rtp_dest;
    struct sockaddr_in udp_rtcp_dest;

    uint16_t rtp_seq;
    uint32_t ssrc;
    int has_sent_keyframe;
    uint32_t last_rtp_timestamp;
    uint32_t start_ms;

    char req_buf[1024];
    int  req_len;
    uint32_t session_id;
};

static struct rtsp_client clients[MAX_CLIENTS];

/* Cache SPS and PPS per channel */
static uint8_t sps_cache[MAX_CHANNELS][64];
static int     sps_len[MAX_CHANNELS];
static uint8_t pps_cache[MAX_CHANNELS][32];
static int     pps_len[MAX_CHANNELS];
static char    sps_b64[MAX_CHANNELS][128];
static char    pps_b64[MAX_CHANNELS][64];

/* Frame ingestion buffer */
#define FRAME_BUF_SIZE (128 * 1024)
static uint8_t frame_buf[FRAME_BUF_SIZE];

#define MAX_FRAME_SIZE (96 * 1024)
static uint8_t reassembled_frame[MAX_CHANNELS][MAX_FRAME_SIZE];
static int     reassembled_len[MAX_CHANNELS];
static int     expected_frame_size[MAX_CHANNELS];

#define MAX_GOP_FRAMES 180
#define GOP_DATA_BUF_SIZE (512 * 1024)
#define ACTIVE_GOP_CHANNELS 2

struct gop_frame_desc {
    uint32_t offset;
    uint32_t len;
    uint32_t rel_ts; // relative 90k timestamp from base_ts
};

struct gop_cache {
    uint8_t  data[GOP_DATA_BUF_SIZE];
    struct gop_frame_desc frames[MAX_GOP_FRAMES];
    int      count;
    uint32_t bytes_used;
    uint32_t base_ts;
    int      overflow;
};

static struct gop_cache gops[ACTIVE_GOP_CHANNELS];

/* --- Nova (DVR local daemon) Client --- */
static int nova_fd = -1;
static uint8_t nova_seq = 0;
static uint32_t last_keepalive_sec = 0;

static int connect_nova(void) {
    if (nova_fd >= 0) {
        sys_close(nova_fd);
        nova_fd = -1;
    }
    nova_fd = sock_socket(AF_INET, SOCK_STREAM, 0);
    if (nova_fd < 0) return -1;

    struct sockaddr_in addr;
    m_memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NOVA_PORT);
    addr.sin_addr.s_addr = 0x0100007f; // 127.0.0.1

    if (sock_connect(nova_fd, &addr, sizeof(addr)) < 0) {
        sys_close(nova_fd);
        nova_fd = -1;
        return -1;
    }

    // 1. Send Login packet (cmd 0x01, user: r00t, pass: 1111)
    // Payload: [0x00, ulen(4), plen(4), 'r','0','0','t', '1','1','1','1'] -> len 11
    const char user[] = "r00t";
    const char pass[] = "1111";
    uint8_t login_buf[32];
    int ulen = m_strlen(user);
    int plen = m_strlen(pass);
    int pay_len = 3 + ulen + plen;
    int total_len = pay_len + 12;

    uint32_t len_be = htonl(total_len - 4);
    m_memcpy(login_buf, &len_be, 4);
    login_buf[4] = 0x00; login_buf[5] = 0xa1; // magic 0xa100
    login_buf[6] = 0x01; // cmd login
    login_buf[7] = 0x00;
    login_buf[8] = nova_seq++ & 0x7f;
    login_buf[9] = total_len & 0xff;
    login_buf[10] = (total_len >> 8) & 0xff;
    login_buf[11] = (total_len >> 16) & 0xff;
    login_buf[12] = 0x00;
    login_buf[13] = (uint8_t)ulen;
    login_buf[14] = (uint8_t)plen;
    m_memcpy(&login_buf[15], user, ulen);
    m_memcpy(&login_buf[15 + ulen], pass, plen);

    sock_send(nova_fd, login_buf, total_len, 0);

    // Read login response (length prefix + response body)
    uint32_t resp_len_be;
    if (sys_read(nova_fd, &resp_len_be, 4) != 4) {
        sys_close(nova_fd);
        nova_fd = -1;
        return -1;
    }
    uint32_t resp_len = htonl(resp_len_be);
    if (resp_len < 8 || resp_len > 1024) {
        sys_close(nova_fd);
        nova_fd = -1;
        return -1;
    }
    uint8_t resp_body[256];
    int rd = 0;
    while (rd < (int)resp_len) {
        int r = sys_read(nova_fd, &resp_body[rd], resp_len - rd);
        if (r <= 0) break;
        rd += r;
    }
    if (rd < 3 || resp_body[2] != 0x02) { // byte offset 2 in body is cmd (offset 6 in full pkt)
        sys_close(nova_fd);
        nova_fd = -1;
        return -1;
    }

    // 2. Query Camera Info (cmd 0x2e)
    total_len = 12;
    len_be = htonl(total_len - 4);
    uint8_t query_buf[16];
    m_memcpy(query_buf, &len_be, 4);
    query_buf[4] = 0x00; query_buf[5] = 0xa1;
    query_buf[6] = 0x2e;
    query_buf[7] = 0x00;
    query_buf[8] = nova_seq++ & 0x7f;
    query_buf[9] = total_len & 0xff;
    query_buf[10] = (total_len >> 8) & 0xff;
    query_buf[11] = (total_len >> 16) & 0xff;
    sock_send(nova_fd, query_buf, 12, 0);

    // 3. Start Live Stream (cmd 0x03, mask=0x0f, fps=30)
    total_len = 16;
    len_be = htonl(total_len - 4);
    uint8_t start_buf[20];
    m_memcpy(start_buf, &len_be, 4);
    start_buf[4] = 0x00; start_buf[5] = 0xa1;
    start_buf[6] = 0x03;
    start_buf[7] = 0x00;
    start_buf[8] = nova_seq++ & 0x7f;
    start_buf[9] = total_len & 0xff;
    start_buf[10] = (total_len >> 8) & 0xff;
    start_buf[11] = (total_len >> 16) & 0xff;
    start_buf[12] = 0x03; // mask: channels 0 and 1 only
    start_buf[13] = 0x00;
    start_buf[14] = 30;   // 30 fps
    start_buf[15] = 0x00;
    sock_send(nova_fd, start_buf, 16, 0);

    set_nonblock(nova_fd);
    return 0;
}

static void send_nova_keepalive(void) {
    if (nova_fd < 0) return;
    int total_len = 12;
    uint32_t len_be = htonl(total_len - 4);
    uint8_t ka_buf[16];
    m_memcpy(ka_buf, &len_be, 4);
    ka_buf[4] = 0x00; ka_buf[5] = 0xa1;
    ka_buf[6] = 0x2a; // keepalive cmd
    ka_buf[7] = 0x00;
    ka_buf[8] = nova_seq++ & 0x7f;
    ka_buf[9] = total_len & 0xff;
    ka_buf[10] = (total_len >> 8) & 0xff;
    ka_buf[11] = (total_len >> 16) & 0xff;
    sock_send(nova_fd, ka_buf, 12, 0);
}

/* --- RTP Packetizer & Broadcast --- */
#define RTP_MTU 1380

static void close_client(struct rtsp_client *c);

static int rtp_send_packet(struct rtsp_client *c, const uint8_t *rtp_payload, int payload_len, int marker, uint32_t rtp_timestamp) {
    uint8_t buf[RTP_MTU + 64];
    int off = 0;

    if (c->transport_type == TRANSPORT_TCP) {
        // 4-byte TCP interleaved header: [0x24][$][channel][len_hi][len_lo]
        buf[0] = 0x24; // '$'
        buf[1] = (uint8_t)c->rtp_ch;
        uint16_t total_rtp_len = htons(12 + payload_len);
        m_memcpy(&buf[2], &total_rtp_len, 2);
        off = 4;
    }

    // 12-byte RTP header
    buf[off + 0] = 0x80; // V=2, P=0, X=0, CC=0
    buf[off + 1] = (marker ? 0x80 : 0x00) | 96; // Payload Type 96 (dynamic H264)
    uint16_t seq_be = htons(c->rtp_seq++);
    m_memcpy(&buf[off + 2], &seq_be, 2);
    uint32_t ts_be = htonl(rtp_timestamp);
    m_memcpy(&buf[off + 4], &ts_be, 4);
    uint32_t ssrc_be = htonl(c->ssrc);
    m_memcpy(&buf[off + 8], &ssrc_be, 4);
    off += 12;

    m_memcpy(&buf[off], rtp_payload, payload_len);
    off += payload_len;

    if (c->transport_type == TRANSPORT_TCP) {
        int res = send_all(c->fd, buf, off);
        if (res < 0) return -1;
    } else if (c->transport_type == TRANSPORT_UDP && c->udp_rtp_fd >= 0) {
        int res = sock_sendto(c->udp_rtp_fd, &buf[4], off - 4, 0, &c->udp_rtp_dest, sizeof(c->udp_rtp_dest));
        if (res < 0) return -1;
    }
    return 0;
}

static uint32_t last_nal_dump_sec[MAX_CHANNELS] = {0};
static uint32_t rtp_start_sec = 0;

static uint32_t get_rtp_timestamp_now(void) {
    struct timeval tv;
    sys_gettimeofday(&tv, NULL);
    if (rtp_start_sec == 0) {
        rtp_start_sec = tv.tv_sec;
    }
    uint32_t delta_sec = (uint32_t)(tv.tv_sec - rtp_start_sec);
    return delta_sec * 90000 + (uint32_t)((tv.tv_usec * 9) / 100);
}

static int send_frame_to_client(struct rtsp_client *c, const uint8_t *frame_data, int frame_len, uint32_t timestamp_90k) {
    int nal_count = 0;
    int nal_starts[32];
    int nal_lens[32];

    int pos = 0;
    while (pos < frame_len - 4) {
        if (frame_data[pos] == 0 && frame_data[pos+1] == 0) {
            int sc_len = 0;
            if (frame_data[pos+2] == 1) {
                sc_len = 3;
            } else if (frame_data[pos+2] == 0 && frame_data[pos+3] == 1) {
                sc_len = 4;
            }
            if (sc_len > 0) {
                if (nal_count > 0) {
                    nal_lens[nal_count - 1] = pos - nal_starts[nal_count - 1];
                }
                if (nal_count < 32) {
                    nal_starts[nal_count] = pos + sc_len;
                    nal_count++;
                }
                pos += sc_len;
                continue;
            }
        }
        pos++;
    }
    if (nal_count > 0) {
        nal_lens[nal_count - 1] = frame_len - nal_starts[nal_count - 1];
    }

    // Send each NAL unit
    for (int n = 0; n < nal_count; n++) {
        const uint8_t *nal_ptr = &frame_data[nal_starts[n]];
        int nlen = nal_lens[n];
        // Trim any trailing zero alignment bytes
        while (nlen > 0 && nal_ptr[nlen - 1] == 0) {
            nlen--;
        }
        if (nlen <= 0) continue;
        int is_last_nal = (n == nal_count - 1);

        if (nlen <= RTP_MTU) {
            // Single NAL unit packet
            if (rtp_send_packet(c, nal_ptr, nlen, is_last_nal, timestamp_90k) < 0) {
                return -1;
            }
        } else {
            // FU-A fragmentation
            uint8_t nal_hdr = nal_ptr[0];
            uint8_t nal_ref_idc = nal_hdr & 0x60;
            uint8_t nal_type = nal_hdr & 0x1f;

            const uint8_t *payload_ptr = nal_ptr + 1;
            int remaining = nlen - 1;
            int is_first = 1;

            while (remaining > 0) {
                int chunk_size = remaining > RTP_MTU ? RTP_MTU : remaining;
                int is_last_chunk = (remaining <= RTP_MTU);

                uint8_t fu_buf[RTP_MTU + 2];
                fu_buf[0] = nal_ref_idc | 28; // FU indicator (type 28)
                fu_buf[1] = (is_first ? 0x80 : 0x00) | (is_last_chunk ? 0x40 : 0x00) | nal_type; // FU header
                m_memcpy(&fu_buf[2], payload_ptr, chunk_size);

                int marker = (is_last_chunk && is_last_nal) ? 1 : 0;
                if (rtp_send_packet(c, fu_buf, chunk_size + 2, marker, timestamp_90k) < 0) {
                    return -1;
                }

                payload_ptr += chunk_size;
                remaining -= chunk_size;
                is_first = 0;
            }
        }
    }
    return 0;
}

static void broadcast_h264_frame(int channel, int is_keyframe, uint8_t *frame_data, int frame_len, uint32_t timestamp_90k) {
    // 1. Locate all NAL units in the unswapped frame to check for SPS/PPS/IDR
    int nal_count = 0;
    int nal_starts[32];
    int nal_lens[32];
    int nal_types[32];

    int pos = 0;
    while (pos < frame_len - 4) {
        if (frame_data[pos] == 0 && frame_data[pos+1] == 0) {
            int sc_len = 0;
            if (frame_data[pos+2] == 1) {
                sc_len = 3;
            } else if (frame_data[pos+2] == 0 && frame_data[pos+3] == 1) {
                sc_len = 4;
            }
            if (sc_len > 0) {
                if (nal_count > 0) {
                    nal_lens[nal_count - 1] = pos - nal_starts[nal_count - 1];
                }
                if (nal_count < 32) {
                    nal_starts[nal_count] = pos + sc_len;
                    nal_types[nal_count] = frame_data[pos + sc_len] & 0x1f;
                    nal_count++;
                }
                pos += sc_len;
                continue;
            }
        }
        pos++;
    }
    if (nal_count > 0) {
        nal_lens[nal_count - 1] = frame_len - nal_starts[nal_count - 1];
    }

    // 2. Identify NAL types & Cache SPS/PPS
    int has_idr = 0;
    int has_sps = 0;
    int has_pps = 0;
    for (int i = 0; i < nal_count; i++) {
        int t = nal_types[i];
        int nlen = nal_lens[i];
        const uint8_t *ndata = &frame_data[nal_starts[i]];
        if (t == 7 && nlen < 64 && channel >= 0 && channel < MAX_CHANNELS) { // SPS
            has_sps = 1;
            m_memcpy(sps_cache[channel], ndata, nlen);
            sps_len[channel] = nlen;
            base64_encode(ndata, nlen, sps_b64[channel], sizeof(sps_b64[channel]));
        } else if (t == 8 && nlen < 32 && channel >= 0 && channel < MAX_CHANNELS) { // PPS
            has_pps = 1;
            m_memcpy(pps_cache[channel], ndata, nlen);
            pps_len[channel] = nlen;
            base64_encode(ndata, nlen, pps_b64[channel], sizeof(pps_b64[channel]));
        } else if (t == 5) { // IDR Keyframe
            has_idr = 1;
        }
    }

    // Update GOP Buffer for this channel
    if (channel >= 0 && channel < ACTIVE_GOP_CHANNELS) {
        struct gop_cache *g = &gops[channel];
        if (has_idr) {
            // New GOP begins with IDR
            g->count = 0;
            g->bytes_used = 0;
            g->base_ts = timestamp_90k;
            g->overflow = 0;
        }

        if (!g->overflow) {
            if (g->count < MAX_GOP_FRAMES && (g->bytes_used + (uint32_t)frame_len <= GOP_DATA_BUF_SIZE)) {
                uint32_t f_ts = 0;
                if (g->count == 0) {
                    f_ts = 0;
                } else {
                    uint32_t prev_ts = g->frames[g->count - 1].rel_ts;
                    uint32_t calc_ts = timestamp_90k - g->base_ts;
                    if (calc_ts <= prev_ts) {
                        calc_ts = prev_ts + 3000;
                    }
                    f_ts = calc_ts;
                }
                m_memcpy(&g->data[g->bytes_used], frame_data, frame_len);
                g->frames[g->count].offset = g->bytes_used;
                g->frames[g->count].len = frame_len;
                g->frames[g->count].rel_ts = f_ts;
                g->bytes_used += frame_len;
                g->count++;
            } else {
                g->overflow = 1;
            }
        }
    }

    // Dump latest keyframe to /tmp/live_X.h264 for WebUI / HA snapshots
    if (has_idr && channel >= 0 && channel < MAX_CHANNELS && frame_len > 100) {
        struct timeval tv;
        sys_gettimeofday(&tv, NULL);
        if ((uint32_t)tv.tv_sec - last_nal_dump_sec[channel] >= 1) {
            last_nal_dump_sec[channel] = tv.tv_sec;
            char path[32];
            path[0] = '/'; path[1] = 't'; path[2] = 'm'; path[3] = 'p'; path[4] = '/';
            path[5] = 'l'; path[6] = 'i'; path[7] = 'v'; path[8] = 'e'; path[9] = '_';
            path[10] = '0' + channel;
            path[11] = '.'; path[12] = 'h'; path[13] = '2'; path[14] = '6'; path[15] = '4';
            path[16] = '\0';
            int fd = sys_open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd >= 0) {
                sys_write(fd, frame_data, frame_len);
                sys_close(fd);
            }
        }
    }

    // 3. Dispatch to clients
    for (int c_idx = 0; c_idx < MAX_CLIENTS; c_idx++) {
        struct rtsp_client *c = &clients[c_idx];
        if (c->state != STATE_PLAYING || c->channel != channel) continue;

        // Ensure stream starts with a keyframe so decoder starts cleanly
        if (!c->has_sent_keyframe) {
            if (!has_idr) continue;
            struct timeval tv_start;
            sys_gettimeofday(&tv_start, NULL);
            c->start_ms = (uint32_t)(tv_start.tv_sec * 1000 + tv_start.tv_usec / 1000);
            c->last_rtp_timestamp = 0;
            // Send cached SPS and PPS first if this IDR packet did not contain them
            if (!has_sps && sps_len[channel] > 0) {
                rtp_send_packet(c, sps_cache[channel], sps_len[channel], 0, 0);
            }
            if (!has_pps && pps_len[channel] > 0) {
                rtp_send_packet(c, pps_cache[channel], pps_len[channel], 0, 0);
            }
            c->has_sent_keyframe = 1;
        }

        struct timeval tv_now;
        sys_gettimeofday(&tv_now, NULL);
        uint32_t now_ms = (uint32_t)(tv_now.tv_sec * 1000 + tv_now.tv_usec / 1000);
        uint32_t client_ts = (now_ms - c->start_ms) * 90;
        if (client_ts <= c->last_rtp_timestamp) {
            client_ts = c->last_rtp_timestamp + 3000;
        }
        c->last_rtp_timestamp = client_ts;

        if (send_frame_to_client(c, frame_data, frame_len, client_ts) < 0) {
            close_client(c);
        }
    }
}

/* --- RTSP Request Handling --- */
static int find_cseq(const char *req) {
    const char *p = m_strstr(req, "CSeq:");
    if (!p) p = m_strstr(req, "cseq:");
    if (!p) return 1;
    p += 5;
    while (*p == ' ' || *p == '\t') p++;
    int val = 0;
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
    }
    return val ? val : 1;
}

static int parse_channel_from_url(const char *url) {
    // Matches /live/0, /live/1, /trackID=0 etc.
    const char *p = m_strstr(url, "/live/");
    if (p) {
        p += 6;
        if (*p >= '0' && *p <= '3') return *p - '0';
    }
    p = m_strstr(url, "/trackID=");
    if (p) {
        p += 9;
        if (*p >= '0' && *p <= '3') return *p - '0';
    }
    return 0; // Default CH0
}

static void close_client(struct rtsp_client *c) {
    if (c->fd >= 0) {
        sys_close(c->fd);
        c->fd = -1;
    }
    if (c->udp_rtp_fd >= 0) {
        sys_close(c->udp_rtp_fd);
        c->udp_rtp_fd = -1;
    }
    c->state = STATE_FREE;
    c->req_len = 0;
}

static void handle_rtsp_request(struct rtsp_client *c) {
    c->req_buf[c->req_len] = '\0';
    int cseq = find_cseq(c->req_buf);

    char resp[2048];
    int rlen = 0;

    if (m_strncmp(c->req_buf, "OPTIONS", 7) == 0) {
        const char opt_hdr[] = "RTSP/1.0 200 OK\r\nCSeq: ";
        m_memcpy(&resp[rlen], opt_hdr, m_strlen(opt_hdr)); rlen += m_strlen(opt_hdr);
        // append cseq
        char cs[16];
        int ci = 0; int tcs = cseq;
        if (tcs == 0) cs[ci++] = '0';
        else while (tcs > 0) { cs[ci++] = '0' + (tcs % 10); tcs /= 10; }
        for (int i = ci - 1; i >= 0; i--) resp[rlen++] = cs[i];
        const char opt_tail[] = "\r\nPublic: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, GET_PARAMETER, SET_PARAMETER\r\n\r\n";
        m_memcpy(&resp[rlen], opt_tail, m_strlen(opt_tail)); rlen += m_strlen(opt_tail);
        send_all(c->fd, resp, rlen);

    } else if (m_strncmp(c->req_buf, "DESCRIBE", 8) == 0) {
        c->channel = parse_channel_from_url(c->req_buf);
        int ch = c->channel;

        const char *sps_str = sps_b64[ch][0] ? sps_b64[ch] : "Z0IAHunBYPiAAAAA";
        const char *pps_str = pps_b64[ch][0] ? pps_b64[ch] : "aMqBAgYIAA==";

        char sdp[1024];
        int slen = 0;
        const char sdp_tpl1[] = "v=0\r\no=- 1001 1 IN IP4 0.0.0.0\r\ns=CE-R4S ASTEL-LT Stream\r\nt=0 0\r\nm=video 0 RTP/AVP 96\r\nc=IN IP4 0.0.0.0\r\nb=AS:2048\r\na=rtpmap:96 H264/90000\r\na=fmtp:96 packetization-mode=1;profile-level-id=42001e;sprop-parameter-sets=";
        m_memcpy(&sdp[slen], sdp_tpl1, m_strlen(sdp_tpl1)); slen += m_strlen(sdp_tpl1);
        m_memcpy(&sdp[slen], sps_str, m_strlen(sps_str)); slen += m_strlen(sps_str);
        sdp[slen++] = ',';
        m_memcpy(&sdp[slen], pps_str, m_strlen(pps_str)); slen += m_strlen(pps_str);
        const char sdp_tpl2[] = "\r\na=control:trackID=0\r\n";
        m_memcpy(&sdp[slen], sdp_tpl2, m_strlen(sdp_tpl2)); slen += m_strlen(sdp_tpl2);

        const char desc_hdr[] = "RTSP/1.0 200 OK\r\nCSeq: ";
        m_memcpy(&resp[rlen], desc_hdr, m_strlen(desc_hdr)); rlen += m_strlen(desc_hdr);
        char cs[16]; int ci = 0; int tcs = cseq;
        if (tcs == 0) cs[ci++] = '0';
        else while (tcs > 0) { cs[ci++] = '0' + (tcs % 10); tcs /= 10; }
        for (int i = ci - 1; i >= 0; i--) resp[rlen++] = cs[i];

        const char desc_mid[] = "\r\nContent-Type: application/sdp\r\nContent-Length: ";
        m_memcpy(&resp[rlen], desc_mid, m_strlen(desc_mid)); rlen += m_strlen(desc_mid);
        ci = 0; tcs = slen;
        while (tcs > 0) { cs[ci++] = '0' + (tcs % 10); tcs /= 10; }
        for (int i = ci - 1; i >= 0; i--) resp[rlen++] = cs[i];

        const char desc_end[] = "\r\n\r\n";
        m_memcpy(&resp[rlen], desc_end, m_strlen(desc_end)); rlen += m_strlen(desc_end);
        m_memcpy(&resp[rlen], sdp, slen); rlen += slen;

        send_all(c->fd, resp, rlen);

    } else if (m_strncmp(c->req_buf, "SETUP", 5) == 0) {
        c->channel = parse_channel_from_url(c->req_buf);
        const char *p = m_strstr(c->req_buf, "interleaved=");
        if (!p) {
            // Client requested UDP; signal 461 Unsupported Transport so standard RTSP clients fall back to TCP
            const char u_hdr[] = "RTSP/1.0 461 Unsupported Transport\r\nCSeq: ";
            m_memcpy(&resp[rlen], u_hdr, m_strlen(u_hdr)); rlen += m_strlen(u_hdr);
            char cs[16]; int ci = 0; int tcs = cseq;
            if (tcs == 0) cs[ci++] = '0';
            else while (tcs > 0) { cs[ci++] = '0' + (tcs % 10); tcs /= 10; }
            for (int i = ci - 1; i >= 0; i--) resp[rlen++] = cs[i];
            const char u_tail[] = "\r\n\r\n";
            m_memcpy(&resp[rlen], u_tail, m_strlen(u_tail)); rlen += m_strlen(u_tail);
            send_all(c->fd, resp, rlen);
            c->req_len = 0;
            return;
        }

        c->transport_type = TRANSPORT_TCP;
        c->rtp_ch = 0;
        c->rtcp_ch = 1;

        p += 12;
        c->rtp_ch = *p - '0';
        if (*(p+1) == '-' && *(p+2) >= '0' && *(p+2) <= '9') {
            c->rtcp_ch = *(p+2) - '0';
        }

        c->state = STATE_READY;

        const char setup_hdr[] = "RTSP/1.0 200 OK\r\nCSeq: ";
        m_memcpy(&resp[rlen], setup_hdr, m_strlen(setup_hdr)); rlen += m_strlen(setup_hdr);
        char cs[16]; int ci = 0; int tcs = cseq;
        if (tcs == 0) cs[ci++] = '0';
        else while (tcs > 0) { cs[ci++] = '0' + (tcs % 10); tcs /= 10; }
        for (int i = ci - 1; i >= 0; i--) resp[rlen++] = cs[i];

        const char setup_tp[] = "\r\nTransport: RTP/AVP/TCP;unicast;interleaved=";
        m_memcpy(&resp[rlen], setup_tp, m_strlen(setup_tp)); rlen += m_strlen(setup_tp);
        resp[rlen++] = '0' + c->rtp_ch;
        resp[rlen++] = '-';
        resp[rlen++] = '0' + c->rtcp_ch;

        const char setup_sess[] = "\r\nSession: ";
        m_memcpy(&resp[rlen], setup_sess, m_strlen(setup_sess)); rlen += m_strlen(setup_sess);
        ci = 0; tcs = c->session_id;
        while (tcs > 0) { cs[ci++] = '0' + (tcs % 10); tcs /= 10; }
        for (int i = ci - 1; i >= 0; i--) resp[rlen++] = cs[i];

        const char setup_end[] = ";timeout=60\r\n\r\n";
        m_memcpy(&resp[rlen], setup_end, m_strlen(setup_end)); rlen += m_strlen(setup_end);

        send_all(c->fd, resp, rlen);

    } else if (m_strncmp(c->req_buf, "PLAY", 4) == 0) {
        c->state = STATE_PLAYING;
        c->rtp_seq = 100;

        const char play_hdr[] = "RTSP/1.0 200 OK\r\nCSeq: ";
        m_memcpy(&resp[rlen], play_hdr, m_strlen(play_hdr)); rlen += m_strlen(play_hdr);
        char cs[16]; int ci = 0; int tcs = cseq;
        if (tcs == 0) cs[ci++] = '0';
        else while (tcs > 0) { cs[ci++] = '0' + (tcs % 10); tcs /= 10; }
        for (int i = ci - 1; i >= 0; i--) resp[rlen++] = cs[i];

        const char play_mid[] = "\r\nRange: npt=0.000-\r\nSession: ";
        m_memcpy(&resp[rlen], play_mid, m_strlen(play_mid)); rlen += m_strlen(play_mid);
        ci = 0; tcs = c->session_id;
        while (tcs > 0) { cs[ci++] = '0' + (tcs % 10); tcs /= 10; }
        for (int i = ci - 1; i >= 0; i--) resp[rlen++] = cs[i];

        const char play_end[] = "\r\n\r\n";
        m_memcpy(&resp[rlen], play_end, m_strlen(play_end)); rlen += m_strlen(play_end);

        send_all(c->fd, resp, rlen);

        // Immediate GOP burst if we have a valid cached GOP
        int ch = c->channel;
        if (ch >= 0 && ch < ACTIVE_GOP_CHANNELS && !gops[ch].overflow && gops[ch].count > 0) {
            struct gop_cache *g = &gops[ch];

            // Send an initial IDR frame for FFmpeg stream probing
            const uint8_t *idr_data = &g->data[g->frames[0].offset];
            int idr_len = g->frames[0].len;
            send_frame_to_client(c, idr_data, idr_len, 0);

            // Burst entire GOP sequence (IDR + P frames) for the actual video decoder
            for (int f = 0; f < g->count; f++) {
                const uint8_t *fdata = &g->data[g->frames[f].offset];
                int flen = g->frames[f].len;
                uint32_t fts = g->frames[f].rel_ts;
                if (send_frame_to_client(c, fdata, flen, fts) < 0) {
                    close_client(c);
                    return;
                }
            }
            struct timeval tv_now;
            sys_gettimeofday(&tv_now, NULL);
            uint32_t now_ms = (uint32_t)(tv_now.tv_sec * 1000 + tv_now.tv_usec / 1000);
            uint32_t last_ts = g->frames[g->count - 1].rel_ts;
            c->start_ms = now_ms - (last_ts / 90);
            c->last_rtp_timestamp = last_ts;
            c->has_sent_keyframe = 1;
        } else {
            c->has_sent_keyframe = 0; // Wait for clean I-frame before transmitting frames
        }

    } else if (m_strncmp(c->req_buf, "TEARDOWN", 8) == 0) {
        const char td_hdr[] = "RTSP/1.0 200 OK\r\nCSeq: ";
        m_memcpy(&resp[rlen], td_hdr, m_strlen(td_hdr)); rlen += m_strlen(td_hdr);
        char cs[16]; int ci = 0; int tcs = cseq;
        if (tcs == 0) cs[ci++] = '0';
        else while (tcs > 0) { cs[ci++] = '0' + (tcs % 10); tcs /= 10; }
        for (int i = ci - 1; i >= 0; i--) resp[rlen++] = cs[i];
        resp[rlen++] = '\r'; resp[rlen++] = '\n'; resp[rlen++] = '\r'; resp[rlen++] = '\n';
        send_all(c->fd, resp, rlen);
        close_client(c);
        return;

    } else if (m_strncmp(c->req_buf, "GET_PARAMETER", 13) == 0 || m_strncmp(c->req_buf, "SET_PARAMETER", 13) == 0) {
        const char ping_hdr[] = "RTSP/1.0 200 OK\r\nCSeq: ";
        m_memcpy(&resp[rlen], ping_hdr, m_strlen(ping_hdr)); rlen += m_strlen(ping_hdr);
        char cs[16]; int ci = 0; int tcs = cseq;
        if (tcs == 0) cs[ci++] = '0';
        else while (tcs > 0) { cs[ci++] = '0' + (tcs % 10); tcs /= 10; }
        for (int i = ci - 1; i >= 0; i--) resp[rlen++] = cs[i];
        resp[rlen++] = '\r'; resp[rlen++] = '\n'; resp[rlen++] = '\r'; resp[rlen++] = '\n';
        send_all(c->fd, resp, rlen);
    }

    c->req_len = 0;
}

/* --- Program Entry & Main Event Loop --- */
void _start(void) {
    // Ignore SIGPIPE (13) so client disconnections never crash the daemon
    sys_signal(13, 1);

    print_str("Starting ASTEL-LT Native Micro RTSP Server (port 554)...\n");

    // Initialize client tables
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].state = STATE_FREE;
        clients[i].fd = -1;
        clients[i].udp_rtp_fd = -1;
        clients[i].session_id = 100000 + i;
        clients[i].ssrc = 0x11223344 + i;
    }

    // Initialize SPS / PPS defaults
    for (int ch = 0; ch < MAX_CHANNELS; ch++) {
        m_memcpy(sps_b64[ch], "Z0IAHunBYPiAAAAA", 17);
        m_memcpy(pps_b64[ch], "aMqBAgYIAA==", 13);
    }

    // Create RTSP listening socket
    int rtsp_listen_fd = sock_socket(AF_INET, SOCK_STREAM, 0);
    if (rtsp_listen_fd < 0) {
        print_str("Error: Failed to create RTSP socket\n");
        sys_exit(1);
    }

    int reuse = 1;
    sock_setsockopt(rtsp_listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in serv_addr;
    m_memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(RTSP_PORT);
    serv_addr.sin_addr.s_addr = 0; // 0.0.0.0

    if (sock_bind(rtsp_listen_fd, &serv_addr, sizeof(serv_addr)) < 0) {
        print_str("Error: Failed to bind RTSP port 554\n");
        sys_close(rtsp_listen_fd);
        sys_exit(2);
    }

    if (sock_listen(rtsp_listen_fd, 8) < 0) {
        print_str("Error: Failed to listen on RTSP port 554\n");
        sys_close(rtsp_listen_fd);
        sys_exit(3);
    }
    set_nonblock(rtsp_listen_fd);

    print_str("RTSP server listening on 0.0.0.0:554\n");

    // Connect to local nova daemon (127.0.0.1:7000)
    if (connect_nova() == 0) {
        print_str("Connected to local DVR streaming engine (nova 127.0.0.1:7000)\n");
    } else {
        print_str("Warning: Initial connection to nova failed; will retry in loop\n");
    }

    struct pollfd fds[MAX_CLIENTS + 2];

    while (1) {
        struct timeval now;
        sys_gettimeofday(&now, NULL);

        // Keepalive to nova every 5 seconds
        if ((uint32_t)now.tv_sec - last_keepalive_sec >= 5) {
            last_keepalive_sec = now.tv_sec;
            if (nova_fd < 0) {
                connect_nova();
            } else {
                send_nova_keepalive();
            }
        }

        // Setup poll fds
        int nfds = 0;

        // fd 0: RTSP listen
        fds[nfds].fd = rtsp_listen_fd;
        fds[nfds].events = POLLIN;
        fds[nfds].revents = 0;
        int idx_listen = nfds++;

        // fd 1: nova
        int idx_nova = -1;
        if (nova_fd >= 0) {
            fds[nfds].fd = nova_fd;
            fds[nfds].events = POLLIN;
            fds[nfds].revents = 0;
            idx_nova = nfds++;
        }

        // RTSP client fds
        int client_map[MAX_CLIENTS];
        for (int i = 0; i < MAX_CLIENTS; i++) {
            client_map[i] = -1;
            if (clients[i].state != STATE_FREE && clients[i].fd >= 0) {
                fds[nfds].fd = clients[i].fd;
                fds[nfds].events = POLLIN;
                fds[nfds].revents = 0;
                client_map[i] = nfds++;
            }
        }

        int poll_res = sys_poll(fds, nfds, 50); // 50ms timeout
        if (poll_res < 0) {
            continue;
        }

        // 1. Check for incoming RTSP client connections
        if (fds[idx_listen].revents & POLLIN) {
            struct sockaddr_in cli_addr;
            int clen = sizeof(cli_addr);
            int new_fd = sock_accept(rtsp_listen_fd, &cli_addr, &clen);
            if (new_fd >= 0) {
                set_nonblock(new_fd);
                int sndbuf = 128 * 1024;
                sock_setsockopt(new_fd, 1 /*SOL_SOCKET*/, 7 /*SO_SNDBUF*/, &sndbuf, sizeof(sndbuf));
                int nodelay = 1;
                sock_setsockopt(new_fd, 6 /*IPPROTO_TCP*/, 1 /*TCP_NODELAY*/, &nodelay, sizeof(nodelay));
                int slot = -1;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].state == STATE_FREE) {
                        slot = i;
                        break;
                    }
                }
                if (slot >= 0) {
                    clients[slot].state = STATE_INIT;
                    clients[slot].fd = new_fd;
                    clients[slot].req_len = 0;
                    clients[slot].has_sent_keyframe = 0;
                    clients[slot].rtp_seq = 100;
                    clients[slot].channel = 0;
                } else {
                    sys_close(new_fd); // full
                }
            }
        }

        // 2. Check for RTSP requests from existing clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int p_idx = client_map[i];
            if (p_idx >= 0 && (fds[p_idx].revents & (POLLIN | POLLHUP | POLLERR))) {
                struct rtsp_client *c = &clients[i];
                int r = sys_read(c->fd, &c->req_buf[c->req_len], sizeof(c->req_buf) - 1 - c->req_len);
                if (r <= 0) {
                    close_client(c);
                } else {
                    c->req_len += r;
                    c->req_buf[c->req_len] = '\0';
                    if (m_strstr(c->req_buf, "\r\n\r\n")) {
                        handle_rtsp_request(c);
                    }
                }
            }
        }

        // 3. Check for streaming packets from nova
        if (idx_nova >= 0 && (fds[idx_nova].revents & POLLIN)) {
            // Read 4-byte packet length prefix
            uint32_t len_be;
            int r = sys_read(nova_fd, &len_be, 4);
            if (r != 4) {
                print_str("Nova disconnected. Will reconnect.\n");
                sys_close(nova_fd);
                nova_fd = -1;
                continue;
            }
            uint32_t plen = htonl(len_be);
            if (plen < 8 || plen > FRAME_BUF_SIZE) {
                // Out of sync or packet too large, drain and reconnect
                print_str("Invalid packet len from nova: ");
                print_int(plen);
                print_str("\n");
                sys_close(nova_fd);
                nova_fd = -1;
                continue;
            }

            // Read the packet payload in full
            int rd = 0;
            while (rd < (int)plen) {
                int n = sys_read(nova_fd, &frame_buf[rd], plen - rd);
                if (n <= 0) break;
                rd += n;
            }
            if (rd < (int)plen) {
                sys_close(nova_fd);
                nova_fd = -1;
                continue;
            }

            // Inspect packet header
            // Offset 2 in body (6 in full pkt) is cmd
            uint8_t cmd = frame_buf[2];
            if (cmd == 0x29) { // Video AV Frame
                int chan_id = frame_buf[10];     // pkt[14]
                int is_keyframe = frame_buf[11] & 0x01; // pkt[15]
                uint32_t total_size = (uint32_t)frame_buf[12] | ((uint32_t)frame_buf[13] << 8) | ((uint32_t)frame_buf[14] << 16) | ((uint32_t)frame_buf[15] << 24);
                uint32_t chunk_offset = (uint32_t)frame_buf[16] | ((uint32_t)frame_buf[17] << 8) | ((uint32_t)frame_buf[18] << 16) | ((uint32_t)frame_buf[19] << 24);

                if (chan_id >= 0 && chan_id < MAX_CHANNELS) {
                    if (chunk_offset == 0) {
                        int header_len = (frame_buf[8] & 0xc0) ? 0x24 : 0x1c; // 0x28-4 or 0x20-4
                        if ((int)plen > header_len) {
                            uint8_t *raw_frag = &frame_buf[header_len];
                            int frag_len = plen - header_len;

                            // In-place 32-bit word unswapping
                            uint32_t *words = (uint32_t*)raw_frag;
                            int n_words = frag_len / 4;
                            for (int w = 0; w < n_words; w++) words[w] = __builtin_bswap32(words[w]);

                            // Skip subheader bytes to start code 00 00 00 01
                            int sc = -1;
                            for (int i = 0; i < frag_len - 4; i++) {
                                if (raw_frag[i] == 0 && raw_frag[i+1] == 0 && raw_frag[i+2] == 0 && raw_frag[i+3] == 1) {
                                    sc = i;
                                    break;
                                }
                            }
                            if (sc >= 0) {
                                raw_frag += sc;
                                frag_len -= sc;
                            }

                            if (frag_len < MAX_FRAME_SIZE) {
                                m_memcpy(reassembled_frame[chan_id], raw_frag, frag_len);
                                reassembled_len[chan_id] = frag_len;
                                expected_frame_size[chan_id] = total_size;
                            }
                        }
                    } else {
                        // Continuation chunk: header is 0x1c (28 bytes in body)
                        int header_len = 0x1c;
                        if ((int)plen > header_len && reassembled_len[chan_id] > 0) {
                            uint8_t *raw_frag = &frame_buf[header_len];
                            int frag_len = plen - header_len;

                            uint32_t *words = (uint32_t*)raw_frag;
                            int n_words = frag_len / 4;
                            for (int w = 0; w < n_words; w++) words[w] = __builtin_bswap32(words[w]);

                            if (reassembled_len[chan_id] + frag_len < MAX_FRAME_SIZE) {
                                m_memcpy(&reassembled_frame[chan_id][reassembled_len[chan_id]], raw_frag, frag_len);
                                reassembled_len[chan_id] += frag_len;
                            }
                        }
                    }

                    // Check if complete frame has been assembled
                    if (reassembled_len[chan_id] > 0 && (chunk_offset + plen >= total_size || (chunk_offset == 0 && plen >= total_size))) {
                        uint32_t real_rtp_ts = get_rtp_timestamp_now();
                        broadcast_h264_frame(chan_id, is_keyframe, reassembled_frame[chan_id], reassembled_len[chan_id], real_rtp_ts);
                        reassembled_len[chan_id] = 0;
                    }
                }
            }
        }
    }
}
