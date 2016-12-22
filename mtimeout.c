#define _POSIX_C_SOURCE 1 /* sigaction */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

static int usage(void);
static void sigalarm_handler(int);

static char *file;
static int timeout;
static int pid;

int main(int argc, char **argv)
{
    struct stat st;
    if (argc < 4 ||
        stat(argv[1], &st) != 0 ||
        sscanf(argv[2], "%d", &timeout) != 1) {
        return usage();
    }
    file = argv[1];

    pid = fork();
    if (pid == 0) {
        execvp(argv[3], &argv[3]);
        perror("execv");
        return 2;
    }

    static struct sigaction act = {
        .sa_handler = sigalarm_handler,
        // .sa_sigaction unset, may be union with .sa_handler
        .sa_mask = 0,
        .sa_flags = 0,
        .sa_restorer = NULL,
    };
    if (sigaction(SIGALRM, &act, NULL) != 0) {
        perror("sigaction");
        return 3;
    }

    sigalarm_handler(0);
    alarm(1);

    int status;
    int rc;
    do {
        rc = waitpid(pid, &status, 0);
    } while (rc < 0 && errno == EINTR);
    if (rc < 0) {
        perror("waitpid");
        return 1;
    }
    return WEXITSTATUS(status);
}

static inline int usage(void)
{
    fprintf(stderr, "Usage: mtimeout <file> <timeout> <cmd> [<args> ... ]\n");
    return 1;
}

static void sigalarm_handler(int sig)
{
    static time_t prev_mtime = 0;
    static int ticks_without_update = 0;

    (void)sig;

    alarm(1);

    struct stat st;
    if (stat(file, &st) < 0 || prev_mtime == st.st_mtime) {
        ticks_without_update++;
    } else {
        ticks_without_update = 0;
        prev_mtime = st.st_mtime;
    }

    if (ticks_without_update >= timeout) {
        fprintf(stderr, "mtimeout: timed out, killing process\n");
        kill(pid, SIGTERM);
    }
}
