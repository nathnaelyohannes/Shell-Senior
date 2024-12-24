//Name: Nathnael Yohannes
//Student ID: 121302540


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "command.h"

int execute(struct tree *t) {

    if (t == NULL) return -1;
    char cwd[1024];

    pid_t pid;

    if (t->conjunction == NONE) {

        if (t->argv[0] != NULL) {

            if (strcmp(t->argv[0], "cd") == 0) {

                const char *dir = t->argv[1] ? t->argv[1] : getenv("HOME");
                if (chdir(dir) == -1) perror(dir);
                return 0;
            }

            if (strcmp(t->argv[0], "exit") == 0) exit(0);
        }

        pid = fork();

        if (pid == 0) {

            if (t->input) {

                int fd_in = open(t->input, O_RDONLY);

                if (fd_in == -1) {

                    fprintf(stderr, "Failed to open input file: %s\n", t->input);
                    exit(1);
                }

                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }

            if (t->output) {

                int fd_out = open(t->output, O_WRONLY | O_CREAT | O_TRUNC, 0664);

                if (fd_out == -1) {

                    fprintf(stderr, "Failed to open output file: %s\n", t->output);
                    exit(1);
                }

                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }

            execvp(t->argv[0], t->argv);
            fprintf(stderr, "Failed to execute %s\n", t->argv[0]);
            exit(1);
        }

        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }

    if (t->conjunction == PIPE) {

        if (t->left->output) {

            printf("Ambiguous output redirect.\n");
            return 1;
        }

        if (t->right->input) {

            printf("Ambiguous input redirect.\n");
            return 1;
        }

        int pipefd[2];

        if (pipe(pipefd) == -1) { 
         
         perror("pipe"); exit(1); 
        
        }

        pid_t left_pid = fork();

        if (left_pid == 0) {

            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]); close(pipefd[1]);
            exit(execute(t->left));
        }

        pid_t right_pid = fork();

        if (right_pid == 0) {

            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]); close(pipefd[1]);
            exit(execute(t->right));
        }

        close(pipefd[0]); close(pipefd[1]);
        waitpid(left_pid, NULL, 0);
        waitpid(right_pid, NULL, 0);
        return 0;
    }

    if (t->conjunction == AND) {

        int left_status = execute(t->left);

        if (left_status == 0) 
        return execute(t->right);

        return left_status;
    }

    if (t->conjunction == SUBSHELL) {

        if (getcwd(cwd, sizeof(cwd)) == NULL) {

            perror("getcwd");
            return -1;
        }

        pid = fork();

        if (pid == 0) {

            chdir(cwd);
            exit(execute(t->left));
        }

        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }

    return 0;
}
