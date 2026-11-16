#define _DEFAULT_SOURCE

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static void print_status(int status)
{
    unsigned int bits = (unsigned int)status;

    printf("status=0x%04x  binary=", bits);
    for (int bit = 15; bit >= 0; --bit) {
        if (bit == 7) {
            putchar(' ');
        }
        putchar((bits & (1u << bit)) != 0 ? '1' : '0');
    }
    printf("\nbits[15:8]=0x%02x  bit[7]=%u  bits[6:0]=%u\n",
           (bits >> 8) & 0xffu, (bits >> 7) & 1u, bits & 0x7fu);

    if (WIFEXITED(status)) {
        printf("WIFEXITED=1  WEXITSTATUS=%d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("WIFSIGNALED=1  WTERMSIG=%d  WCOREDUMP=%d\n",
               WTERMSIG(status), WCOREDUMP(status) ? 1 : 0);
    } else if (WIFSTOPPED(status)) {
        printf("WIFSTOPPED=1  WSTOPSIG=%d\n", WSTOPSIG(status));
    } else if (WIFCONTINUED(status)) {
        puts("WIFCONTINUED=1");
    }
}

static pid_t wait_retry(int *status)
{
    pid_t result;
    do {
        result = wait(status);
    } while (result == -1 && errno == EINTR);
    return result;
}

static pid_t waitpid_retry(pid_t child, int *status, int options)
{
    pid_t result;
    do {
        result = waitpid(child, status, options);
    } while (result == -1 && errno == EINTR);
    return result;
}

static int test_wait(const char *label, int exit_code, int fatal_signal)
{
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return -1;
    }
    if (child == 0) {
        if (fatal_signal != 0) {
            raise(fatal_signal);
        }
        _exit(exit_code);
    }

    int status;
    pid_t result = wait_retry(&status);
    if (result == -1) {
        perror("wait");
        return -1;
    }

    printf("\n[%s]\nfork(): child PID=%ld  wait(): return=%ld\n",
           label, (long)child, (long)result);
    print_status(status);
    if (result != child) {
        return -1;
    }
    if (fatal_signal != 0) {
        return WIFSIGNALED(status) && WTERMSIG(status) == fatal_signal ? 0 : -1;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == (exit_code & 0xff) ? 0 : -1;
}

static int test_waitpid(void)
{
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return -1;
    }
    if (child == 0) {
        raise(SIGSTOP);
        for (;;) {
            pause();
        }
    }

    int stopped = 0;
    int continued = 0;
    puts("\n[waitpid(): stop, continue, terminate]");
    for (;;) {
        int status;
        pid_t result = waitpid_retry(child, &status, WUNTRACED | WCONTINUED);
        if (result == -1) {
            perror("waitpid");
            return -1;
        }

        printf("fork(): child PID=%ld  waitpid(): return=%ld\n",
               (long)child, (long)result);
        print_status(status);

        int next_signal;
        if (WIFSTOPPED(status)) {
            stopped = WSTOPSIG(status) == SIGSTOP;
            next_signal = SIGCONT;
        } else if (WIFCONTINUED(status)) {
            continued = 1;
            next_signal = SIGKILL;
        } else {
            return stopped && continued &&
                   WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL ? 0 : -1;
        }

        if (kill(child, next_signal) == -1) {
            perror("kill");
            if (kill(child, SIGKILL) == 0 || errno == ESRCH) {
                waitpid_retry(child, NULL, 0);
            }
            return -1;
        }
    }
}

int main(void)
{
    if (test_wait("wait(): child calls _exit(300)", 300, 0) == -1 ||
        test_wait("wait(): child dies from SIGKILL", 0, SIGKILL) == -1 ||
        test_waitpid() == -1) {
        fputs("FAIL: unexpected child state\n", stderr);
        return EXIT_FAILURE;
    }

    errno = 0;
    pid_t result = wait_retry(NULL);
    if (result != -1 || errno != ECHILD) {
        fputs("FAIL: wait() should report ECHILD\n", stderr);
        return EXIT_FAILURE;
    }
    printf("\n[wait(): no child left]\nwait(): return=%ld  errno=ECHILD (%d)\n",
           (long)result, errno);

    puts("\n[Synthetic status: SIGABRT with the core flag set]");
    print_status(0x80 | SIGABRT);

    puts("\nPASS: observed the expected wait and waitpid results");
    return EXIT_SUCCESS;
}
