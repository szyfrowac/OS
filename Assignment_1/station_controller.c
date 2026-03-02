#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

float base_frequency = 800.0;
float current_freq = 800.0;

int quit(int argc, char* argv[]) {
    printf("Exiting station controller...\n");
    exit(0);
}

int get_freq(int argc, char* argv[]) {
    printf("Current frequency: %.2f MHz\n", current_freq);
    return 0;
}

int set_freq(int argc, char* argv[]) {
    if(argc < 2) {
        printf("Usage: set_freq <frequency>\n");
        return 1;
    }
    float freq = atof(argv[1]);
    if(freq <= 0) {
        printf("Please enter a positive frequency.\n");
        return 1;
    }
    current_freq = freq;
    printf("Frequency set to %.2f MHz\n", current_freq);
    return 0;
}

void sigInthandler(int sig) {
    signal(SIGINT, sigInthandler);
    printf("[WARNING] Carrier Interrupt Signal Received, reset frequency to default safe mode(800 MHz), continue normal operation and only quit command can exit the shell.\n");
    current_freq = base_frequency;
    printf("station-controller:$> ");
    fflush(stdout);
}

typedef int (*function_ptr)(int argc, char* argv[]);

typedef struct {
    char* name;
    function_ptr func;
} command;

command internal[] = {
    {"quit", quit},
    {"get_freq", get_freq},
    {"set_freq", set_freq}
};

int execute_command(char* name, int argc, char* argv[]) {

    int fork_start = 0;

    for(int i = 0; i < sizeof(internal) / sizeof(command); i++) {
        if(strcmp(name, internal[i].name) == 0) {
            return internal[i].func(argc, argv);
        }
    }

    char *functions[] = {"ping", "top"};
   
    for (int i = 0; i < sizeof(functions) / sizeof(char*); i++) {
        if(strcmp(name, functions[i]) == 0) {
            fork_start = 1;
            break;
        }
    }

    if(fork_start) {
        pid_t f = fork();
        if(f == 0) {
            if(strcmp(name, "ping") == 0) {
                if(argc < 2) {
                    printf("Usage: ping <hostname>\n");
                    exit(1);
                }
                char *ping_args[] = {"ping", "-c", "4", argv[1], NULL};
                execvp("ping", ping_args);
                fflush(stdout);
            } else {
                execvp(name, argv);
                fflush(stdout);
            }
            exit(1);

        } else if (f == -1) {
            printf("Fork failed\n");
            return 1;

        } else {
            int status;
            wait(&status);
        }

        return 0;
    } else {
        printf("Unknown command: %s\n", name);
        return 1;
    }


    // const char* path[] = {"/bin/", "/usr/bin/"};
    // char full_path[2][256];
    // for(int i = 0; i < 2; i++) {
    //     snprintf(full_path[i], sizeof(full_path[i]), "%s%s", path[i], name);
    //     if(access(full_path[i], X_OK) == 0) {

    //     }
    // }
    
    // int status = execve(name, argv, NULL);
    // if(status == -1) {
    //     printf("Unknown command: %s\n", name);
    // }
    // return -1;
}

int main (void) {

    current_freq = base_frequency;

    signal(SIGINT, sigInthandler);

    while(1) {
        // char *working_dir = getcwd(NULL, 0);

        // if(working_dir == NULL) {
        //     perror("getcwd() error");
        //     return 1;
        // }

        printf("station-controller:$> ");
        // free(working_dir);

        char buf[1024];
        fgets(buf, sizeof(buf), stdin);

        char* delim = "\n -";
        char* token = strtok(buf, delim);
        char* argv[100];
        int argc = 0;

        while(token != NULL) {
            argv[argc++] = token;
            token = strtok(NULL, delim);
            // printf("Token: %s\n", argv[argc-1]);
        }
        argv[argc] = NULL;
        int internal_command = execute_command(argv[0], argc, argv);

    }

}