#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 80 
#define MAX_ARGS (MAX_LINE/2 + 1) 

int main(void) {
    char input[MAX_LINE];
    char *args[MAX_ARGS];
    int should_run = 1;

    while (should_run) {
        /* 1. PRINT PROMPT */
        printf("mini_bash> ");
        fflush(stdout);

        /* 2. GET USER INPUT */
        if (fgets(input, MAX_LINE, stdin) == NULL) break;
        input[strcspn(input, "\n")] = '\0';

        /* 3. PARSE INPUT INTO TOKENS */
        int i = 0;
        char *token = strtok(input, " \t\r\n");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        args[i] = NULL; 

        if (args[0] == NULL) continue;

        /* 4. BUILT-IN COMMAND: EXIT */
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        /* 5. BUILT-IN COMMAND: CD */
        if (strcmp(args[0], "cd") == 0) {
            /* If no argument provided, or too many, handle error */
            if (args[1] == NULL) {
                fprintf(stderr, "mini_bash: expected argument to \"cd\"\n");
            } else {
                if (chdir(args[1]) != 0) {
                    perror("mini_bash");
                }
            }
            continue; /* Skip fork and continue to next prompt */
        }

        /* 6. EXTERNAL COMMANDS: FORK AND EXECUTE */
        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Fork Failed\n");
            return 1;
        } 
        else if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                printf("mini_bash: command not found: %s\n", args[0]);
            }
            exit(1);
        } 
        else {
            wait(NULL);
        }
    }

    return 0;
}