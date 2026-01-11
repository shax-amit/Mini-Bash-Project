#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE 80 /* The maximum length command */
#define MAX_ARGS (MAX_LINE/2 + 1) /* The maximum number of arguments */

int main(void) {
    char input[MAX_LINE];
    char *args[MAX_ARGS];
    int should_run = 1;

    while (should_run) {
        /* 1. PRINT PROMPT */
        printf("mini_bash> ");
        fflush(stdout);

        /* 2. GET USER INPUT */
        if (fgets(input, MAX_LINE, stdin) == NULL) {
            break;
        }

        /* 3. REMOVE NEWLINE CHARACTER */
        input[strcspn(input, "\n")] = '\0';

        /* 4. PARSE INPUT INTO TOKENS */
        int i = 0;
        char *token = strtok(input, " \t\r\n");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        args[i] = NULL; /* NULL terminate the list of arguments */

        /* 5. CHECK FOR EXIT COMMAND */
        if (args[0] != NULL && strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        /* 6. DISPLAY PARSING RESULT (TEMPORARY FOR TESTING) */
        if (args[0] != NULL) {
            printf("Executing: %s\n", args[0]);
            /* Next step will implement fork and execvp here */
        }
    }

    return 0;
}