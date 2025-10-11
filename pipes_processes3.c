#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Check 
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <grep_argument>\n", argv[0]);
        exit(1);
    }

    int pipe1[2];  
    int pipe2[2];  
    // Create first pipe
    if (pipe(pipe1) == -1) {
        perror("pipe1 failed");
        exit(1);
    }
    
    // Create second pipe
    if (pipe(pipe2) == -1) {
        perror("pipe2 failed");
        exit(1);
    }

    // Create first child process (grep)
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid1 == 0) {
        // First child process (grep)
        
        // Create second child process (sort) from first child
        pid_t pid2 = fork();
        if (pid2 == -1) {
            perror("fork failed");
            exit(1);
        }

        if (pid2 == 0) {
            // Second child process (sort)
            
            // Close unused pipe ends
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[1]);  // Close write end of pipe2
            
            // Redirect stdin to read from pipe2
            dup2(pipe2[0], STDIN_FILENO);
            close(pipe2[0]);
            
            // Execute sort
            execlp("sort", "sort", NULL);
            perror("execlp sort failed");
            exit(1);
        } else {
            // First child process (grep)
            
            // Close unused pipe ends
            close(pipe1[1]);  // Close write end of pipe1
            close(pipe2[0]);  // Close read end of pipe2
            
            // Redirect stdin to read from pipe1
            dup2(pipe1[0], STDIN_FILENO);
            close(pipe1[0]);
            
            // Redirect stdout to write to pipe2
            dup2(pipe2[1], STDOUT_FILENO);
            close(pipe2[1]);
            
           
            execlp("grep", "grep", argv[1], NULL);
            perror("execlp grep failed");
            exit(1);
        }
    } else {
        
        close(pipe1[0]);  
        close(pipe2[0]);  
        close(pipe2[1]);  
        
        
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);
        
        
        execlp("cat", "cat", "scores", NULL);
        perror("execlp cat failed");
        exit(1);
    }

    return 0;
}