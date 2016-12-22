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
static void signal_handler(int);

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
        .sa_handler = signal_handler,
        // .sa_sigaction unset, may be union with .sa_handler
        .sa_mask = 0,
        .sa_flags = 0,
        .sa_restorer = NULL,
    };

    // 1. SIGALRM is used internally by mtimeout
    // 2. most other signals are forwarded (SIGINT, SIGTERM, etc.)
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGTRAP, &act, NULL);
    sigaction(SIGABRT, &act, NULL);
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    /* SIGKILL - cannot foward */
    sigaction(SIGUSR1, &act, NULL);
    /* SIGSEGV - our fault */
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGPIPE, &act, NULL);
    sigaction(SIGALRM, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGSTKFLT, &act, NULL);
    /* SIGCHLD - <cmd> exitted */
    sigaction(SIGCONT, &act, NULL);
    /* SIGSTOP - cannot forward */
    sigaction(SIGTSTP, &act, NULL);
    sigaction(SIGTTIN, &act, NULL);
    sigaction(SIGTTOU, &act, NULL);
    sigaction(SIGURG, &act, NULL);
    sigaction(SIGXCPU, &act, NULL);
    sigaction(SIGXFSZ, &act, NULL);
    sigaction(SIGVTALRM, &act, NULL);
    sigaction(SIGPROF, &act, NULL);
    sigaction(SIGWINCH, &act, NULL);
    sigaction(SIGPOLL, &act, NULL);
    sigaction(SIGPWR, &act, NULL);
    sigaction(SIGSYS, &act, NULL);

    signal_handler(SIGALRM);
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

static void signal_handler(int sig)
{
    static time_t prev_mtime = 0;
    static int ticks_without_update = 0;

    if (sig == SIGALRM) {
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
        
    } else {
        // forward to child
        kill(pid, sig);
    }

    

}
