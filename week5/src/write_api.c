#include <stdio.h>
#include <unistd.h>

int main(void)
{
    const char msg[] = "Hello, Linux syscall!\n";
    ssize_t ret = write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    if (ret == -1) {
        perror("write");
        return 1;
    }

    printf("API: write returned %ld bytes\n", (long)ret);
    return ret == (ssize_t)(sizeof(msg) - 1) ? 0 : 1;
}
