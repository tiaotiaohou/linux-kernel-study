#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int open_destination(const char *source, const char *target, mode_t mode)
{
    int flags = O_WRONLY | O_CREAT | O_NONBLOCK;
    int output = open(target, flags, mode);
    if (output != -1 || errno != EISDIR) {
        return output;
    }

    int directory = open(target, O_PATH | O_DIRECTORY);
    if (directory == -1) {
        return -1;
    }

    const char *last_slash = strrchr(source, '/');
    const char *name = last_slash == NULL ? source : last_slash + 1;
    output = openat(directory, name, flags, mode);
    int open_error = errno;
    close(directory);
    errno = open_error;
    return output;
}

static int copy_contents(int input, int output)
{
    char buffer[65536];

    for (;;) {
        ssize_t count = read(input, buffer, sizeof buffer);
        if (count == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (count == 0) {
            return 0;
        }

        size_t written = 0;
        while (written < (size_t)count) {
            ssize_t chunk = write(output, buffer + written, (size_t)count - written);
            if (chunk == -1) {
                if (errno == EINTR) {
                    continue;
                }
                return -1;
            }
            if (chunk == 0) {
                errno = EIO;
                return -1;
            }
            written += (size_t)chunk;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s SOURCE DESTINATION\n", argv[0]);
        return EXIT_FAILURE;
    }

    int input = open(argv[1], O_RDONLY | O_NONBLOCK);
    if (input == -1) {
        perror(argv[1]);
        return EXIT_FAILURE;
    }

    int output = -1;
    int status = EXIT_FAILURE;
    struct stat source_info;
    struct stat target_info;

    if (fstat(input, &source_info) == -1) {
        perror("mycp: fstat source");
        goto cleanup;
    }
    if (!S_ISREG(source_info.st_mode)) {
        fputs("mycp: source must be a regular file\n", stderr);
        goto cleanup;
    }

    mode_t permissions = source_info.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    output = open_destination(argv[1], argv[2], permissions);
    if (output == -1) {
        perror(argv[2]);
        goto cleanup;
    }
    if (fstat(output, &target_info) == -1) {
        perror("mycp: fstat destination");
        goto cleanup;
    }
    if (!S_ISREG(target_info.st_mode)) {
        fputs("mycp: destination must resolve to a regular file\n", stderr);
        goto cleanup;
    }
    if (source_info.st_dev == target_info.st_dev &&
        source_info.st_ino == target_info.st_ino) {
        fputs("mycp: source and destination refer to the same file\n", stderr);
        goto cleanup;
    }

    /* Check file identity before truncating the destination. */
    if (ftruncate(output, 0) == -1) {
        perror("mycp: truncate destination");
        goto cleanup;
    }
    if (copy_contents(input, output) == -1) {
        perror("mycp: copy");
        goto cleanup;
    }
    status = EXIT_SUCCESS;

cleanup:
    if (output != -1 && close(output) == -1) {
        perror("mycp: close destination");
        status = EXIT_FAILURE;
    }
    if (close(input) == -1) {
        perror("mycp: close source");
        status = EXIT_FAILURE;
    }
    return status;
}
