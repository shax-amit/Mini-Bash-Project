#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 
#define MAX_ARGS (MAX_LINE/2 + 1) 

/* FUNCTION TO CLEAN INPUT: Removes non-printable characters */
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

/* Helper function: Checks if the path exists and has execution permissions */
int is_executable(char *path) {
    return access(path, X_OK) == 0;
}

int main(void) {
    char input[MAX_LINE];
    char last_command[MAX_LINE];
    int has_history = 0;
    char *args[MAX_ARGS];
    int should_run = 1;

    memset(last_command, 0, MAX_LINE);

    /* Main execution loop */
    while (should_run) {
        printf("mini_bash> ");
        fflush(stdout);

        memset(input, 0, MAX_LINE);
        if (fgets(input, MAX_LINE, stdin) == NULL) break;
        input[strcspn(input, "\n")] = '\0';
        sanitize_input(input);

        if (strlen(input) == 0) continue;

        /* HISTORY LOGIC (!!) */
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

        /* PARSE INPUT into tokens */
        int i = 0;
        char *token = strtok(input, " \t\r\n");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t\r\n");
        }
        args[i] = NULL; 

        /* CHECK FOR PIPE (|) */
        int pipe_idx = -1;
        for (int k = 0; args[k] != NULL; k++) {
            if (strcmp(args[k], "|") == 0) {
                pipe_idx = k;
                break;
            }
        }

        if (pipe_idx != -1) {
            /* PIPE HANDLING */
            args[pipe_idx] = NULL;
            char **left_args = args;
            char **right_args = &args[pipe_idx + 1];

            int fd[2];
            pipe(fd);

            if (fork() == 0) {
                /* Left child: write to pipe */
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(left_args[0], left_args);
                exit(1);
            }

            if (fork() == 0) {
                /* Right child: read from pipe */
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(right_args[0], right_args);
                exit(1);
            }

            close(fd[0]);
            close(fd[1]);
            wait(NULL);
            wait(NULL);
            continue;
        }

        /* CHECK FOR REDIRECTION (>) */
        char *outfile = NULL;
        for (int k = 0; args[k] != NULL; k++) {
            if (strcmp(args[k], ">") == 0) {
                if (args[k+1] != NULL) {
                    outfile = args[k+1];
                    args[k] = NULL;
                }
                break;
            }
        }

        /* BUILT-IN COMMANDS */
        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) fprintf(stderr, "mini_bash: expected argument\n");
            else if (chdir(args[1]) != 0) perror("mini_bash");
            continue;
        }

        /* SINGLE COMMAND EXECUTION */
        pid_t pid = fork();
        if (pid == 0) {
            /* Child Process */
            if (outfile != NULL) {
                int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("mini_bash: open");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            char path[1024];
            int found = 0;

            /* Step A: Search in the HOME directory */
            char *home = getenv("HOME"); 
            if (home != NULL) {
                snprintf(path, sizeof(path), "%s/%s", home, args[0]);
                if (is_executable(path)) {
                    found = 1;
                    execv(path, args); 
                }
            }

            /* Step B: Search in /bin directory if not found in HOME */
            if (!found) {
                snprintf(path, sizeof(path), "/bin/%s", args[0]);
                if (is_executable(path)) {
                    found = 1;
                    execv(path, args);
                }
            }

            /* Step C: Handle command not found */
            if (!found) {
                fprintf(stderr, "[%s]: Unknown Command\n", args[0]);
                exit(127);
            }
        } else if (pid > 0) {
            /* Parent Process */
            int status;
            waitpid(pid, &status, 0); 

            if (WIFEXITED(status)) {
                int return_code = WEXITSTATUS(status);
                printf("Command executed successfully with Return Code: %d\n", return_code);
            }
        } else {
            perror("fork failed");
        }
    }
    return 0;
}