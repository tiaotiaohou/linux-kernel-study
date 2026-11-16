#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

enum { MAX_ARGS = 64 };

static int split_args(char *line, char *argv[])
{
    const char *delimiters = " \t\r\n";
    int argc = 0;

    for (char *word = strtok(line, delimiters); word != NULL;
         word = strtok(NULL, delimiters)) {
        if (argc == MAX_ARGS) {
            fprintf(stderr, "mini-shell: at most %d words per command\n", MAX_ARGS);
            return -1;
        }
        argv[argc++] = word;
    }
    argv[argc] = NULL;
    return argc;
}

static int change_directory(int argc, char *argv[])
{
    if (argc > 2) {
        fputs("usage: cd [directory]\n", stderr);
        return 2;
    }

    const char *directory = argc == 2 ? argv[1] : getenv("HOME");
    if (directory == NULL || *directory == '\0') {
        fputs("cd: HOME is not set\n", stderr);
        return 1;
    }
    if (chdir(directory) == -1) {
        perror("cd");
        return 1;
    }
    return 0;
}

static int run_command(char *argv[])
{
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return 1;
    }
    if (child == 0) {
        execvp(argv[0], argv);
        int exit_code = errno == ENOENT ? 127 : 126;
        perror(argv[0]);
        _exit(exit_code);
    }

    /* 这个 Shell 每次只启动一个子进程，因此用 wait 即可。 */
    int status;
    pid_t finished;
    do {
        finished = wait(&status);
    } while (finished == -1 && errno == EINTR);

    if (finished == -1) {
        perror("wait");
        return 1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return WIFSIGNALED(status) ? 128 + WTERMSIG(status) : 1;
}

int main(void)
{
    char *line = NULL;
    size_t capacity = 0;
    char *argv[MAX_ARGS + 1];
    int interactive = isatty(STDIN_FILENO);
    int last_status = 0;

    for (;;) {
        if (interactive) {
            fputs("mini-shell> ", stdout);
            fflush(stdout);
        }
        if (getline(&line, &capacity, stdin) == -1) {
            if (!feof(stdin)) {
                perror("getline");
                last_status = 1;
            }
            break;
        }

        int argc = split_args(line, argv);
        if (argc <= 0) {
            if (argc < 0) {
                last_status = 2;
            }
            continue;
        }
        if (strcmp(argv[0], "exit") == 0) {
            if (argc == 1) {
                break;
            }
            fputs("usage: exit\n", stderr);
            last_status = 2;
            continue;
        }
        if (strcmp(argv[0], "cd") == 0) {
            last_status = change_directory(argc, argv);
        } else {
            last_status = run_command(argv);
        }
    }

    free(line);
    return last_status;
}
