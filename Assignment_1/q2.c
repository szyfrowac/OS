#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>


int main() {
    int n, k, r;

    printf("Enter the number of processes: ");
    scanf("%d", &k);

    printf("Enter the time interval for display (in seconds): ");
    scanf("%d", &n);

    printf("Enter the iterations after which the program should terminate (in seconds): ");
    scanf("%d", &r);

    // int fd[2];

    // if(pipe(fd) == -1) {
    //     printf("Pipe failed\n");
    //     return 1;
    // }

    int f = fork();
    char command[256];
    int x = 0;
    if(f == 0) {
        // child process
        int iterations = 0;
        while(1) {
            for(iterations = 0; iterations < r; iterations++) {
                snprintf(command, sizeof(command), "ps -e -o pid,user,comm,time,%%mem --sort=-%%mem | head -n %d", k + 1);
                printf("Displaying process information...\n");
                system(command);
                sleep(n);
            }
            // printf("Child process terminating after %d iterations.\n", r);
            printf("Enter the input : ");
            scanf("%d", &x);

            // close(fd[1]);
            // read(fd[0], &x, sizeof(x));
            // close(fd[0]);

            if(x == -2) {
                printf("Terminating the child program...\n");
                exit(0);
            } else if (x == -1) {
                iterations = 0;
            } else {
                int status = kill(x, SIGKILL);
                if(status == -1) {
                    printf("Failed to kill process with PID %d. Please check the PID and try again.\n", x);
                } else {
                    printf("Successfully killed process with PID %d.\n", x);
                }
            }
        }
    } else if (f == -1) {
        printf("Fork failed\n");
        return 1;
    } else {
        // parent process
        // close(fd[0]);
        // write(fd[1], &x, sizeof(x));
        // close(fd[1]);
        wait(NULL); // wait for child process to finish
        // printf("Terminating the parent program...\n");
        // kill(f, SIGKILL);
    }

    return 0;
}

