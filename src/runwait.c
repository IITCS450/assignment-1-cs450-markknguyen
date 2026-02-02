//imports
#include "common.h"


#include <sys/wait.h>
#include <time.h>

#include <unistd.h>

static void usage(const char *a) {
    fprintf(stderr, "usage: %s <cmd> [args]\n", a);
    exit(1);
}

static double diff(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

int main(int c, char **v) {
    if (c < 2) usage(v[0]);


    struct timespec t1, t2;

    clock_gettime(CLOCK_MONOTONIC, &t1);

    pid_t pid = fork();
    if (pid < 0) {
        DIE("fork");
    } else if (pid == 0) {
        //child runs command


        execvp(v[1], v + 1);
        DIE("execvp");
    }

    //parent waits
    int st;
    waitpid(pid, &st, 0);

    clock_gettime(CLOCK_MONOTONIC, &t2);

    double elapsed = diff(t1, t2);

    if (WIFEXITED(st)) {


        printf("pid=%d elapsed=%.3f exit=%d\n", pid, elapsed, WEXITSTATUS(st));
    } else if (WIFSIGNALED(st)) {

        
        printf("pid=%d elapsed=%.3f signal=%d\n", pid, elapsed, WTERMSIG(st));
    }

    return 0;
}
