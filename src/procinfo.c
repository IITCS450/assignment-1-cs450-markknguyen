//imports

#include "common.h"


#include <ctype.h>
#include <stdio.h>


#include <unistd.h>

static void usage(const char *a) {
    fprintf(stderr, "usage: %s <pid>\n", a);
    exit(1);
}

static int isnum(const char *s) {
    if (!*s) return 0;


    for (; *s; s++)
        if (!isdigit(*s)) return 0;
    return 1;
}

int main(int c, char **v) {
    if (c != 2 || !isnum(v[1])) usage(v[0]);

    const char *pid = v[1];
    char path[256];
    FILE *fp;


    char state  = '?';
    int ppid = 0;
    unsigned long ut = 0, st = 0 ;
    long rss = 0;
    char cmd[1024]  = "";

    //read /proc/pid/stat
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    fp = fopen(path, "r");


    if (!fp) {
        if (errno  == ENOENT)
            DIE_MSG("process notfound");
        else if (errno == EACCES)


            DIE_MSG("perms denied");
        else
            DIE("fopen stat");
    }

    char buf[2048];
    if (!fgets(buf, sizeof(buf), fp))
        DIE("fgets stat");
    fclose(fp);

    //find end of comm field
    char *p = strrchr(buf, ')');

    if (!p) DIE_MSG("bad stat format");

    //parse: state ppid ... utime stime
    int n = sscanf(p + 2, "%c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu",
        &state, &ppid, &ut, &st);
    if (n < 4) DIE_MSG("parse error");

    //read /proc/pid/status for vmrss
    snprintf(path, sizeof(path), "/proc/%s/status", pid) ;
    fp = fopen(path, "r");
    if (!fp) DIE("fopen status");

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%ld", &rss);


            break;
        }
    }
    fclose(fp);

    //read the /proc/pid/cmdline


    snprintf(path, sizeof(path), "/proc/%s/cmdline", pid);
    fp = fopen(path, "r");


    if (!fp) DIE("fopen cmdline");

    size_t len = fread(cmd, 1, sizeof(cmd) - 1, fp);
    fclose(fp);

    //replace nulls with spaces
    for (size_t i = 0; i < len; i++) {
        if (cmd[i] == '\0') cmd[i] = ' ';
    }
    if (len > 0 && cmd[len - 1] == ' ')
        cmd[len - 1] = '\0';

    if (cmd[0] == '\0')
        snprintf(cmd, sizeof(cmd), "[kernel]");

    //calc cpu time
    long tps = sysconf(_SC_CLK_TCK);


    unsigned long ticks = ut + st;
    double secs = (double)ticks / tps;

    //output IMPORTANT
    printf("PID:%s\n", pid);
    printf("State:%c\n", state) ;
    printf("PPID:%d\n", ppid);


    printf("Cmd:%s\n", cmd);


    printf("CPU:%lu %.3f\n", ticks, secs);
    printf("VmRSS:%ld\n", rss) ;


    return 0;
}
