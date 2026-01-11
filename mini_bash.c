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
    char last_command[MAX_LINE]; /* Buffer to store the last command */
    int has_history = 0;         /* Flag to check if history exists */
    char *args[MAX_ARGS];
    int should_run = 1;

    while (should_run) {
        printf("mini_bash> ");
        fflush(stdout);

        if (fgets(input, MAX_LINE, stdin) == NULL) break;
        input[strcspn(input, "\n")] = '\0';

        /* 1. CHECK FOR HISTORY COMMAND (!!) */
        if (strcmp(input, "!!") == 0) {
            if (!has_history) {
                printf("No commands in history.\n");
                continue;
            }
            /* Use the saved command instead of '!!' */
            strcpy(input, last_command);
            printf("%s\n", input); /* Show the command being repeated */
        } else {
            /* 2. SAVE CURRENT INPUT TO HISTORY */
            strcpy(last_command, input);
            has_history = 1;
        }

        /* 3. PARSE INPUT INTO TOKENS */
        int i = 0;
        char *token = strtok(input, " \t\r\n");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        args[i] = NULL; 

        if (args[0] == NULL) continue;

        /* 4. CHECK FOR BACKGROUND EXECUTION */
        int background = 0;
        if (i > 0 && strcmp(args[i-1], "&") == 0) {
            background = 1;
            args[i-1] = NULL;
        }

        /* 5. BUILT-IN COMMANDS: EXIT & CD */
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "mini_bash: expected argument to \"cd\"\n");
            } else {
                if (chdir(args[1]) != 0) perror("mini_bash");
            }
            continue;
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
            if (background == 0) {
                wait(NULL);
            } else {
                printf("[Process running in background with PID: %d]\n", pid);
            }
        }
    }

    return 0;
}