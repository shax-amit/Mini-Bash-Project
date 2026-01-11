#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> /* Required for open() */

#define MAX_LINE 80 
#define MAX_ARGS (MAX_LINE/2 + 1) 

void sanitize_input(char *str) {
    int i = 0, j = 0;
    while (str[i] != '\0') {
        if ((unsigned char)str[i] >= 32 && (unsigned char)str[i] <= 126) {
            str[j++] = str[i];
        }
        i++;
    }
    str[j] = '\0';
}

int main(void) {
    char input[MAX_LINE];
    char last_command[MAX_LINE];
    int has_history = 0;
    char *args[MAX_ARGS];
    int should_run = 1;

    memset(last_command, 0, MAX_LINE);

    while (should_run) {
        printf("mini_bash> ");
        fflush(stdout);

        memset(input, 0, MAX_LINE);
        if (fgets(input, MAX_LINE, stdin) == NULL) break;
        input[strcspn(input, "\n")] = '\0';
        sanitize_input(input);

        if (strlen(input) == 0) continue;

        if (strcmp(input, "!!") == 0) {
            if (!has_history) {
                printf("No commands in history.\n");
                continue;
            }
            strcpy(input, last_command);
            printf("%s\n", input);
        } else {
            strcpy(last_command, input);
            has_history = 1;
        }

        /* 1. PARSE INPUT */
        int i = 0;
        char *token = strtok(input, " \t\r\n");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        args[i] = NULL; 

        /* 2. CHECK FOR REDIRECTION (>) */
        char *outfile = NULL;
        for (int k = 0; args[k] != NULL; k++) {
            if (strcmp(args[k], ">") == 0) {
                if (args[k+1] != NULL) {
                    outfile = args[k+1];
                    args[k] = NULL; /* Terminate args before '>' */
                }
                break;
            }
        }

        /* 3. BUILT-INS */
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) fprintf(stderr, "mini_bash: expected argument\n");
            else if (chdir(args[1]) != 0) perror("mini_bash");
            continue;
        }

        /* 4. EXECUTION */
        pid_t pid = fork();
        if (pid == 0) {
            /* CHILD PROCESS */
            if (outfile != NULL) {
                /* Redirect stdout to file */
                int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("mini_bash");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (execvp(args[0], args) == -1) {
                printf("mini_bash: command not found: %s\n", args[0]);
            }
            exit(1);
        } else if (pid > 0) {
            wait(NULL);
        }
    }
    return 0;
}