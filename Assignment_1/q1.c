/*
 * Parent process:
 *   - Takes input 'n' from the user
 *   - Has a fixed integer array {3,15,4,6,7,17,9,2}
 *   - Randomly picks an element x, prints it, sends it to child via unnamed pipe
 *   - Sleeps for x%n seconds
 *   - Stops when every element has been visited at least once, or on SIGINT (Ctrl+C)
 *
 * Child process:
 *   - Receives x from the pipe
 *   - Prints all factors of x (including 1 and x)
 *   - Sleeps for time(NULL)%n seconds
 *   - Loops until pipe is closed
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>


void run_child(int read_fd, int n)
{
    int x;
    while (read(read_fd, &x, sizeof(int)) == sizeof(int)) {

  
        printf("[Child]  Factors of %d: ", x);
        for (int i = 1; i <= x; i++) {
            if (x % i == 0) {
                printf("%d ", i);
            }
        }
        printf("\n");
        fflush(stdout);

        unsigned int sleep_time = (unsigned int)(time(NULL) % n);
        printf("[Child]  Sleeping for %u second(s).\n\n", sleep_time);
        fflush(stdout);
        sleep(sleep_time);
    }

    close(read_fd);
    printf("[Child]  Pipe closed. Exiting.\n");
    exit(0);
}


int main(void)
{
    int arr[]  = {3, 15, 4, 6, 7, 17, 9, 2};
    int arr_sz = (int)(sizeof(arr) / sizeof(arr[0]));

    int n;
    printf("Enter n (> 0): ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        fprintf(stderr, "Invalid input. n must be a positive integer.\n");
        return 1;
    }

    int pipefd[2];  
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }


    if (pid == 0) {
        close(pipefd[1]);     
        run_child(pipefd[0], n);  
    }

    close(pipefd[0]);   

    srand(time(NULL));


    /* Block SIGINT so we can check it manually each loop iteration */
    sigset_t blocked, old_mask;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGINT);
    sigprocmask(SIG_BLOCK, &blocked, &old_mask);

    int visited[sizeof(arr) / sizeof(arr[0])];
    memset(visited, 0, sizeof(visited));
    int visited_count = 0;
    int interrupted   = 0;

    printf("[Parent] Starting loop. Array size = %d, n = %d\n\n", arr_sz, n);

    while (visited_count < arr_sz && !interrupted) {

        /* Check if SIGINT is pending (Ctrl+C was pressed) */
        sigset_t pending;
        sigpending(&pending);
        if (sigismember(&pending, SIGINT)) {
            interrupted = 1;
            break;
        }

 
        int idx = rand() % arr_sz;
        int x   = arr[idx];

 
        if (!visited[idx]) {
            visited[idx] = 1;
            visited_count++;
        }

        printf("[Parent] Selected x = %d  (index %d) | visited %d/%d\n",
               x, idx, visited_count, arr_sz);
        fflush(stdout);


        if (write(pipefd[1], &x, sizeof(int)) == -1) {
            perror("[Parent] write");
            break;
        }

        unsigned int sleep_time = (unsigned int)(x % n);
        printf("[Parent] Sleeping for %u second(s).\n", sleep_time);
        fflush(stdout);
        sleep(sleep_time);
    }
    close(pipefd[1]);

    if (interrupted)
        printf("\n[Parent] SIGINT received. Shutting down.\n");
    else
        printf("[Parent] All %d elements visited. Shutting down.\n", arr_sz);

    waitpid(pid, NULL, 0);
    printf("[Parent] Child has exited. Done.\n");

    return 0;
}