#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <signal.h>


struct History {
    char list[6][256];
    int size;
};

void pushInHistory(struct History *history, char *str) {
    int size = history -> size;
    if(size < 6) {
        
        strcpy(history->list[size], str);
        history -> size++;
    } else {
        for(int i = 0; i < 5; i++) {
            strcpy(history -> list[i], history -> list[i+1]);
        }

        strcpy(history -> list[5], str);
    }
}

void printHistory(struct History *history) {
    int p = history -> size;

    for(int i = p-2; i >= 0; i--) {
        printf("%s\n", history -> list[i]);
    }
}

void sigint(int sig_num) {
    printf("\n");
    exit(sig_num);
}

int getPiped(char *str, char** pipedCmds) {
    int count = 0;
    while(1) {
        pipedCmds[count] = strsep(&str, "|");
        if(pipedCmds[count] == NULL)
            break;
        
        count++;
    }

    return count;
}

void spaceBreak(char *str, char **arrString) {
    int i = 0;
    while(1) {
        arrString[i] = strsep(&str, " ");

        if(arrString[i] == NULL)
            break;
        
        // get env variable
        if(arrString[i][0] == '$') {
            char *s = getenv(arrString[i]+1);
            arrString[i] = s;
        }
        if(strlen(arrString[i]) == 0)
            i--;
        i++;
    }
}

int envVar(char *str) {
    if(strstr(str, "=")) {
        char *key = strsep(&str, "=");
        char *value = strsep(&str, "=");

        setenv(key, value, 1);
        return 1;
    }

    return 0;
}

int cmd_history(struct History *history, char *str) {

    int t = strcmp(str, "cmd_history");
    if(t == 0) {
        printHistory(history);
        return 1;
    }

    return 0;
}

void update_ps(int *processes, int *status, int *counter) {
    for(int i = 0; i < *counter; i++) {
        int s;
        int pid = waitpid(processes[i], &s, WNOHANG);
        
        if(pid == 0) status[i] = 1;
        else status[i] = 0;
    }
}

int ps_history(char* str, int *processes, int *status, int *counter) {
    if(strcmp(str, "ps_history") == 0) {
        for(int i = 0; i < *counter; i++) {
            printf("%d", processes[i]);
            if(status[i] == 1)
                printf(" RUNNING\n");
            else    
                printf(" STOPPED\n");
        }

        return 1;
    }

    return 0;
}

int processString(char *str, char **cmd1, char **cmd2, struct History *history, int *processes, int *counter) {

    if(str[0] == ' ')
        return 0;

    if(envVar(str))
        return 0;

    char *piped[2];
    int count = getPiped(str, piped);

    if(count == 1) {
        spaceBreak(str, cmd1);
    } else if(count == 2) {
        spaceBreak(piped[0], cmd1);
        spaceBreak(piped[1], cmd2);
    } else {
        printf("Parsing Unsuccessful\n");
        return -1;
    }

    return count;
} 

void execute(char **cmd, struct History *history, int *processes, int *status, int *counter) {
    pid_t pid = fork();

    int bg = 0;
    if(cmd[0][0] == '&') {
        bg = 1;
        cmd[0]++;
    }
    
    if(pid < 0) {
        printf("\nChild creation failed\n");
        return;
    } else if(pid == 0) {

        if(cmd_history(history, cmd[0])) {
            exit(0);
        }

        if(ps_history(cmd[0], processes, status, counter)) {
            exit(0);
        }

        execvp(cmd[0], cmd);
        printf("Illegal command\n");
        exit(0);
    } else {

        // background process
        if(!bg)
            waitpid(pid, NULL, 0);

        processes[*counter] = pid;
        status[*counter] = 0;
        (*counter)++;
    }
}

void executePipes(char **cmd1, char **cmd2, struct History *history, int *processes, int *status, int *counter) {
    int pipefd[2];
    pid_t p1, p2;

    if(pipe(pipefd) < 0) {
        printf("\nIssues with piping\n");
        return;
    }

    p1 = fork();
    if(p1 < 0) {
        printf("Issues with forking first child\n");
        return;
    }

    if(p1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if(cmd_history(history, cmd1[0])) {
            exit(0);
        }

        if(ps_history(cmd1[0], processes, status, counter)) {
            exit(0);
        }

        else if(execvp(cmd1[0], cmd1) < 0) {
            printf("\nIssues in executing command1\n");
            exit(0);
        }
    } else {
        processes[*counter] = p1;
        status[*counter] = 0;
        (*counter)++;
        p2 = fork();

        if(p2 < 0) {
            printf("Issues with forking second child\n");
            return;
        } 

        if(p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);

            if(cmd_history(history, cmd2[0])) {
                // printf("Works in pipe\n");
                exit(0);
            }

            if(ps_history(cmd2[0], processes, status, counter)) {
                // printf("PS Works in pipe\n");
                exit(0);
            }

            if(execvp(cmd2[0], cmd2) < 0) {
                printf("\nIssues in executing command2\n");
                exit(0);
            }
        } else {
            processes[*counter] = p2;
            status[*counter] = 0;
            (*counter)++;

            wait(NULL);
            close(pipefd[1]);
            wait(NULL);
        }
    }
}

int main() {
    // printf("Hello World \n");

    int MAX_LIMIT = 256;
    char input[MAX_LIMIT];
    char *cmd1[100], *cmd2[100];

    int processes[1000];
    int status[1000];
    int counter = 0;

    // ctrl + c
    signal(SIGINT, sigint);
    
    struct History history;
    history.size = 0;

    while(true) {
        int st;
        int pid = waitpid(-1, &st, WNOHANG);

        // print directory
        char cwd[1024]; 
        getcwd(cwd, sizeof(cwd));
        printf("%s~$ ", cwd);


        // take input and strip the last /n
        fgets(input, MAX_LIMIT, stdin);
        if(input[0] == '\n')
            continue;
        input[strcspn(input, "\n")] = 0;
        
        pushInHistory(&history, input);

        // process the input
        int pipes = processString(input, cmd1, cmd2, &history, processes, &counter);

        // execute stuff 
        update_ps(processes, status, &counter);
        if(pipes == 1) {
            execute(cmd1, &history, processes, status, &counter);
        } else if(pipes == 2) {
            executePipes(cmd1, cmd2, &history, processes, status, &counter);
        }
    }
}