#define NULL ((void*)0)

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

#define __NR_SYSCALL_BASE 0x900000
#define __NR_exit         (__NR_SYSCALL_BASE + 1)
#define __NR_read         (__NR_SYSCALL_BASE + 3)
#define __NR_write        (__NR_SYSCALL_BASE + 4)
#define __NR_close        (__NR_SYSCALL_BASE + 6)
#define __NR_socketcall   (__NR_SYSCALL_BASE + 102)

#define SYS_SOCKET  1
#define SYS_CONNECT 3

#define AF_INET     2
#define SOCK_STREAM 1

struct in_addr {
    uint32_t s_addr;
};

struct sockaddr_in {
    uint16_t sin_family;
    uint16_t sin_port;
    struct in_addr sin_addr;
    uint8_t  sin_zero[8];
};

void *memset(void *s, int c, unsigned long n) {
    uint8_t *p = (uint8_t*)s;
    while (n--) *p++ = (uint8_t)c;
    return s;
}

void *memcpy(void *dst, const void *src, unsigned long n) {
    uint8_t *d = (uint8_t*)dst;
    const uint8_t *s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
    return dst;
}

unsigned long strlen(const char *s) {
    unsigned long len = 0;
    while (s && s[len]) len++;
    return len;
}

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

static inline long sys_close(int fd) {
    register long r0 __asm__("r0") = fd;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_close), "0"(r0) : "memory");
    return r0;
}

static inline long sys_socketcall(int call, unsigned long *args) {
    register long r0 __asm__("r0") = call;
    register long r1 __asm__("r1") = (long)args;
    __asm__ __volatile__("swi %1\n" : "=r"(r0) : "i"(__NR_socketcall), "0"(r0), "r"(r1) : "memory");
    return r0;
}

static void print_str(const char *s) {
    sys_write(1, s, strlen(s));
}

void c_main(long *sp) {
    int argc = (int)sp[0];
    char **argv = (char**)&sp[1];

    if (argc < 3) {
        print_str("Usage: dvr_auth <user> <pass>\n");
        sys_exit(1);
    }

    const char *user = argv[1];
    const char *pass = argv[2];
    int ulen = strlen(user);
    int plen = strlen(pass);

    if (ulen == 0 || ulen > 32 || plen > 32) {
        print_str("AUTH_FAIL\n");
        sys_exit(1);
    }

    unsigned long sock_args[3] = { AF_INET, SOCK_STREAM, 0 };
    int fd = sys_socketcall(SYS_SOCKET, sock_args);
    if (fd < 0) {
        print_str("ERR_SOCKET\n");
        sys_exit(2);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = ((7000 & 0xff) << 8) | ((7000 >> 8) & 0xff); // htons(7000)
    addr.sin_addr.s_addr = 0x0100007f; // 127.0.0.1
    memset(addr.sin_zero, 0, 8);

    unsigned long conn_args[3] = { fd, (unsigned long)&addr, sizeof(addr) };
    if (sys_socketcall(SYS_CONNECT, conn_args) < 0) {
        sys_close(fd);
        print_str("ERR_CONNECT\n");
        sys_exit(3);
    }

    int size = 12 + 3 + ulen + plen;
    uint8_t buf[128];
    memset(buf, 0, sizeof(buf));

    int len_be = size - 4;
    buf[0] = (len_be >> 24) & 0xff;
    buf[1] = (len_be >> 16) & 0xff;
    buf[2] = (len_be >> 8) & 0xff;
    buf[3] = len_be & 0xff;

    buf[4] = 0x00;
    buf[5] = 0xa1;
    buf[6] = 0x01; // cmd login
    buf[7] = 0x00;
    buf[8] = 0x00;
    buf[9] = size & 0xff;
    buf[10] = (size >> 8) & 0xff;
    buf[11] = (size >> 16) & 0xff;

    buf[12] = 0x00;
    buf[13] = ulen;
    buf[14] = plen;
    memcpy(buf + 15, user, ulen);
    memcpy(buf + 15 + ulen, pass, plen);

    if (sys_write(fd, buf, size) != size) {
        sys_close(fd);
        print_str("ERR_WRITE\n");
        sys_exit(4);
    }

    uint8_t resp[32];
    int n = sys_read(fd, resp, 32);
    sys_close(fd);

    if (n >= 8 && resp[6] == 0x02 && resp[7] == 0x00) {
        print_str("AUTH_OK\n");
        sys_exit(0);
    }

    print_str("AUTH_FAIL\n");
    sys_exit(1);
}

__attribute__((naked)) void _start(void) {
    __asm__ __volatile__(
        "mov r0, sp\n"
        "b c_main\n"
    );
}
