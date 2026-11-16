#define _DEFAULT_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>

/* Return 1 for a complete record, 0 at EOF, or -1 on error. */
static int read_record(int fd, struct utmp *record)
{
    unsigned char *buffer = (unsigned char *)record;
    size_t filled = 0;

    while (filled < sizeof *record) {
        ssize_t count = read(fd, buffer + filled, sizeof *record - filled);
        if (count > 0) {
            filled += (size_t)count;
        } else if (count == 0) {
            if (filled == 0) {
                return 0;
            }
            errno = EIO;
            return -1;
        } else if (errno != EINTR) {
            return -1;
        }
    }
    return 1;
}

static int print_login(const struct utmp *record, const char *time_format)
{
    time_t login_time = record->ut_tv.tv_sec;
    struct tm local_time;
    char timestamp[64];

    if (localtime_r(&login_time, &local_time) == NULL ||
        strftime(timestamp, sizeof timestamp, time_format, &local_time) == 0) {
        fputs("mywho: could not format the login time\n", stderr);
        return -1;
    }

    /* A full utmp field may have no terminating NUL. */
    printf("%-8.*s %-12.*s %s",
           (int)sizeof record->ut_user, record->ut_user,
           (int)sizeof record->ut_line, record->ut_line, timestamp);
    if (record->ut_host[0] != '\0') {
        printf(" (%.*s)", (int)sizeof record->ut_host, record->ut_host);
    }
    putchar('\n');
    return 0;
}

int main(void)
{
    setlocale(LC_ALL, "");
    const char *time_locale = setlocale(LC_TIME, NULL);
    const char *time_format = "%Y-%m-%d %H:%M";
    if (strcmp(time_locale, "C") == 0 || strcmp(time_locale, "POSIX") == 0) {
        time_format = "%b %e %H:%M";
    }

    int fd = open(UTMP_FILE, O_RDONLY);
    if (fd == -1) {
        perror("mywho: open " UTMP_FILE);
        return EXIT_FAILURE;
    }

    struct utmp record;
    int read_status;
    int status = EXIT_SUCCESS;

    while ((read_status = read_record(fd, &record)) == 1) {
        if (record.ut_type != USER_PROCESS || record.ut_user[0] == '\0') {
            continue;
        }
        /* Signal 0 checks whether the login process still exists. */
        if (record.ut_pid > 0 && kill(record.ut_pid, 0) == -1 && errno == ESRCH) {
            continue;
        }
        if (print_login(&record, time_format) == -1) {
            status = EXIT_FAILURE;
            break;
        }
    }
    if (read_status == -1) {
        perror("mywho: read " UTMP_FILE);
        status = EXIT_FAILURE;
    }
    if (close(fd) == -1) {
        perror("mywho: close " UTMP_FILE);
        status = EXIT_FAILURE;
    }
    if (fflush(stdout) == EOF || ferror(stdout)) {
        fputs("mywho: could not write the output\n", stderr);
        status = EXIT_FAILURE;
    }
    return status;
}
