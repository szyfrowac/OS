// mygrep.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s PATTERN file1 [file2 ...]\n", argv[0]);
        return 1;
    }

    int p1[2], p2[2], p3[2];
    if (pipe(p1) == -1) die("pipe p1");
    if (pipe(p2) == -1) die("pipe p2");
    if (pipe(p3) == -1) die("pipe p3");

    // 1) grep -nH PATTERN files...
    pid_t c1 = fork();
    if (c1 == -1) die("fork grep");
    if (c1 == 0) {
        // stdout -> p1 write
        if (dup2(p1[1], STDOUT_FILENO) == -1) die("dup2 grep stdout");
        close(p1[0]); close(p1[1]);
        close(p2[0]); close(p2[1]);
        close(p3[0]); close(p3[1]);

        // Build argv for execvp: grep -nH pattern file1 file2 ...
        char **gargv = malloc(sizeof(char*) * (argc + 3));
        if (!gargv) die("malloc");
        int k = 0;
        gargv[k++] = "grep";
        gargv[k++] = "-nH";
        gargv[k++] = argv[1];          // PATTERN
        for (int i = 2; i < argc; i++)  // files
            gargv[k++] = argv[i];
        gargv[k] = NULL;

        execvp("grep", gargv);
        die("execvp grep");
    }

    // 2) cut -d: -f2
    pid_t c2 = fork();
    if (c2 == -1) die("fork cut");
    if (c2 == 0) {
        // stdin <- p1 read, stdout -> p2 write
        if (dup2(p1[0], STDIN_FILENO) == -1) die("dup2 cut stdin");
        if (dup2(p2[1], STDOUT_FILENO) == -1) die("dup2 cut stdout");
        close(p1[0]); close(p1[1]);
        close(p2[0]); close(p2[1]);
        close(p3[0]); close(p3[1]);

        char *cargv[] = {"cut", "-d:", "-f2", NULL};
        execvp("cut", cargv);
        die("execvp cut");
    }

    // 3) sort -n
    pid_t c3 = fork();
    if (c3 == -1) die("fork sort");
    if (c3 == 0) {
        // stdin <- p2 read, stdout -> p3 write
        if (dup2(p2[0], STDIN_FILENO) == -1) die("dup2 sort stdin");
        if (dup2(p3[1], STDOUT_FILENO) == -1) die("dup2 sort stdout");
        close(p1[0]); close(p1[1]);
        close(p2[0]); close(p2[1]);
        close(p3[0]); close(p3[1]);

        char *sargv[] = {"sort", "-n", NULL};
        execvp("sort", sargv);
        die("execvp sort");
    }

    // 4) uniq
    pid_t c4 = fork();
    if (c4 == -1) die("fork uniq");
    if (c4 == 0) {
        // stdin <- p3 read, stdout -> terminal (default)
        if (dup2(p3[0], STDIN_FILENO) == -1) die("dup2 uniq stdin");
        close(p1[0]); close(p1[1]);
        close(p2[0]); close(p2[1]);
        close(p3[0]); close(p3[1]);

        char *uargv[] = {"uniq", NULL};
        execvp("uniq", uargv);
        die("execvp uniq");
    }

    // Parent: close all pipe fds and wait
    close(p1[0]); close(p1[1]);
    close(p2[0]); close(p2[1]);
    close(p3[0]); close(p3[1]);

    int status;
    while (wait(&status) > 0) { }
    return 0;
}