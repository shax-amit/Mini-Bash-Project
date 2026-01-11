#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_LINE 80 
#define MAX_ARGS (MAX_LINE/2 + 1) 

/* FUNCTION TO CLEAN INVISIBLE CHARACTERS OR NON-ASCII */
void sanitize_input(char *str) {
    int i = 0, j = 0;
    while (str[i] != '\0') {
        /* Keep only printable ASCII characters (32-126) */
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

        /* 1. SANITIZE INPUT (Remove Hebrew/Garbage chars) */
        sanitize_input(input);

        if (strlen(input) == 0) continue;

        /* 2. CHECK FOR HISTORY (!!) */
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

        /* 3. PARSE INPUT */
        int i = 0;
        char *token = strtok(input, " \t\r\n");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        args[i] = NULL; 

        if (args[0] == NULL) continue;

        /* 4. BUILT-INS & FORK (Same as before) */
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }
        
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) fprintf(stderr, "mini_bash: expected argument\n");
            else if (chdir(args[1]) != 0) perror("mini_bash");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
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