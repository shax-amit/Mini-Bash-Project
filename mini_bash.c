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

        /* 3. REMOVE NEWLINE CHARACTER */
        input[strcspn(input, "\n")] = '\0';

        /* 4. PARSE INPUT INTO TOKENS */
        int i = 0;
        char *token = strtok(input, " \t\r\n");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        args[i] = NULL; 

        /* 5. CHECK FOR EMPTY INPUT */
        if (args[0] == NULL) continue;

        /* 6. CHECK FOR EXIT COMMAND */
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        /* 7. FORK A CHILD PROCESS */
        pid_t pid = fork();

        if (pid < 0) {
            /* Error occurred */
            fprintf(stderr, "Fork Failed\n");
            return 1;
        } 
        else if (pid == 0) {
            /* 8. CHILD PROCESS: EXECUTE COMMAND */
            if (execvp(args[0], args) == -1) {
                printf("mini_bash: command not found: %s\n", args[0]);
            }
            exit(1); /* Exit child if execvp fails */
        } 
        else {
            /* 9. PARENT PROCESS: WAIT FOR CHILD */
            wait(NULL);
        }
    }

    return 0;
}