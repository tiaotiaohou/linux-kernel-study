#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

static long raw_write(int fd, const void *buf, size_t count)
{
    long ret;
    __asm__ __volatile__(
        "syscall"
        : "=a"(ret)
        : "0"((long)SYS_write), "D"((long)fd), "S"(buf), "d"(count)
        : "rcx", "r11", "cc", "memory"
    );
    return ret;
}

int main(void)
{
    const char msg[] = "Hello, Linux syscall!\n";
    long ret = raw_write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    if (ret < 0) {
        fprintf(stderr, "raw write failed: %ld\n", ret);
        return 1;
    }

    printf("ASM: write returned %ld bytes\n", ret);
    return ret == (long)(sizeof(msg) - 1) ? 0 : 1;
}
