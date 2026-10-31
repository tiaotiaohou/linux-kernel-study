#include <stdio.h>
#include <sys/syscall.h>
#include "menu.h"

static int Write(int argc, char *argv[])
{
    const char msg[] = "Hello, Linux syscall!\n";
    long ret;
    (void)argc;
    (void)argv;

    __asm__ __volatile__(
        "int $0x80"
        : "=a"(ret)
        : "0"((long)SYS_write), "b"(1L), "c"(msg),
          "d"((unsigned long)(sizeof(msg) - 1))
        : "cc", "memory"
    );

    printf("ASM: write returned %ld bytes\n", ret);
    fflush(stdout);
    return ret == (long)(sizeof(msg) - 1) ? 0 : 1;
}

int main(void)
{
    SetPrompt("MenuOS>>");
    MenuConfig("version", "week6 write syscall lab", NULL);
    MenuConfig("write", "invoke write using int 0x80", Write);
    ExecuteMenu();
    return 0;
}
