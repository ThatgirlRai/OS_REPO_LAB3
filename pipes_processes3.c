#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <grep_argument>\n", argv[0]);
        exit(1);
    }

    int pipe1[2];  // cat -> grep
    int pipe2[2];  // grep -> sort

    // Create both pipes
    if (pipe(pipe1) == -1) {
        perror("pipe1 failed");
        exit(1);
    }
    
    if (pipe(pipe2) == -1) {
        perror("pipe2 failed");
        exit(1);
    }

    // Fork child 1 (cat)
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid1 == 0) {
        // Child 1: cat scores
        // Close unused pipe ends
        close(pipe1[0]);
        close(pipe2[0]);
        close(pipe2[1]);
        
        // Redirect stdout to pipe1 write end
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);
        
        execlp("cat", "cat", "scores", NULL);
        perror("execlp cat failed");
        exit(1);
    }

    // Fork child 2 (grep)
    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid2 == 0) {
        // Child 2: grep <argument>
        // Close unused pipe ends
        close(pipe1[1]);
        close(pipe2[0]);
        
        // Redirect stdin to pipe1 read end
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);
        
        // Redirect stdout to pipe2 write end
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe2[1]);
        
        execlp("grep", "grep", argv[1], NULL);
        perror("execlp grep failed");
        exit(1);
    }

    // Fork child 3 (sort)
    pid_t pid3 = fork();
    if (pid3 == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid3 == 0) {
        // Child 3: sort
        // Close unused pipe ends
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[1]);
        
        // Redirect stdin to pipe2 read end
        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);
        
        execlp("sort", "sort", NULL);
        perror("execlp sort failed");
        exit(1);
    }

    // Parent process
    // Close all pipe ends (parent doesn't use any)
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    // Wait for all children to prevent zombies
    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}