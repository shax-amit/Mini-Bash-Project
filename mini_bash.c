#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80 
#define MAX_ARGS (MAX_LINE/2 + 1) 

/* Helper function: Checks if the path exists and has execution permissions [cite: 34, 95] */
int is_executable(char *path) {
    return access(path, X_OK) == 0; 
}

/* Manual Search Logic: HOME -> /bin [cite: 29-36, 106-109] */
void resolve_and_execute(char **args) {
    char path[1024];
    char *home = getenv("HOME"); // [cite: 31-32]

    if (home != NULL) {
        snprintf(path, sizeof(path), "%s/%s", home, args[0]);
        if (is_executable(path)) {
            execv(path, args); // Execute from HOME [cite: 34, 43]
            perror("execv failed"); 
            exit(1);
        }
    }

    snprintf(path, sizeof(path), "/bin/%s", args[0]);
    if (is_executable(path)) {
        execv(path, args); // Execute from /bin [cite: 36, 43]
        perror("execv failed");
        exit(1);
    }

    fprintf(stderr, "[%s]: Unknown Command\n", args[0]); // 
    exit(127);
}

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
        printf("mini_bash> "); // Updated Prompt to use '>' [cite: 19, 100]
        fflush(stdout);

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

        int i = 0;
        char *token = strtok(input, " \t");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t");
        }
        args[i] = NULL; 

        if (strcmp(args[0], "exit") == 0) {
            should_run = 0; 
            continue;
        }
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) fprintf(stderr, "mini_bash: expected argument\n");
            else if (chdir(args[1]) != 0) perror("mini_bash"); // [cite: 25, 27, 94]
            continue;
        }

        int pipe_idx = -1;
        for (int k = 0; args[k] != NULL; k++) {
            if (strcmp(args[k], "|") == 0) {
                pipe_idx = k;
                break;
            }
        }

        if (pipe_idx != -1) {
            args[pipe_idx] = NULL;
            char **left_args = args;
            char **right_args = &args[pipe_idx + 1];

            int fd[2];
            if (pipe(fd) == -1) { perror("pipe"); continue; } // [cite: 96]

            if (fork() == 0) { 
                dup2(fd[1], STDOUT_FILENO); // Redirect stdout to pipe [cite: 96]
                close(fd[0]); close(fd[1]);
                resolve_and_execute(left_args); 
            }
            if (fork() == 0) { 
                dup2(fd[0], STDIN_FILENO); // Redirect stdin from pipe [cite: 96]
                close(fd[0]); close(fd[1]);
                resolve_and_execute(right_args); 
            }
            close(fd[0]); close(fd[1]);
            wait(NULL); wait(NULL); // Parent waits for both children [cite: 44, 93]
            continue;
        }

        pid_t pid = fork(); // [cite: 42, 91]
        if (pid == 0) {
            for (int k = 0; args[k] != NULL; k++) {
                if (strcmp(args[k], ">") == 0) {
                    if (args[k+1] != NULL) {
                        int fd = open(args[k+1], O_WRONLY | O_CREAT | O_TRUNC, 0644); // [cite: 97]
                        if (fd == -1) { perror("open"); exit(1); }
                        dup2(fd, STDOUT_FILENO); // [cite: 96]
                        close(fd);
                        args[k] = NULL;
                    }
                    break;
                }
            }
            resolve_and_execute(args); 
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0); // [cite: 44, 93, 111]
            if (WIFEXITED(status)) {
                printf("Command executed successfully with Return Code: %d\n", WEXITSTATUS(status)); // [cite: 45, 122]
            }
        } else {
            perror("fork failed");
        }
    }
    return 0;
}