// Name: Ludmila Klimes
// Class: CS 3502 
// Term: Spring 2026
// Assignment: Homework 2
// Date: 04/03/26

// mysh - a simple Unix shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

void sigchild(int sig) { // Signal handler (SIGCHLD)
    while (waitpid(-1, NULL, WNOHANG) > 0); // checks for finished child processes
}

int main (void) {
    // Set up signal handler for SIGCHLD
    signal(SIGCHLD, sigchild); 

    
    char line[1024]; // Declare character array to store user input
    char *args[64]; // Declare array of pointers to store tokens
    int arg_count; // Declare integer to store number of tokens (to keep track)

    
    while (1) { // Loop until user enters Ctrl+D
        printf("mysh> "); // Prompt string
        fflush(stdout); // Flush the output buffer

        // Read user input:
        char *check = fgets(line, sizeof(line), stdin); 

        // Check if user entered Ctrl+D
        if (check == NULL) { // If user enters Ctrl+D, exit the shell
            printf("\n");
            break;
        }

        // Remove newline character from the input
        char *newline = strchr(line, '\n'); // Find the newline character
        if (newline) { // If newline character is found, replace with null terminator
            *newline = '\0';
        }

        // Reset the counter and Tokenize the input line
        arg_count = 0; // counter reset
        args[arg_count] = strtok(line, " "); // space delimiter

        // Loop through the tokens and store them in the args array
        while (args[arg_count] != NULL) { // Loop until the end of the tokens
            arg_count++; // Increment the counter
            args[arg_count] = strtok(NULL, " "); // Get the next token
        }

        // Check if the user entered an empty line
        if (arg_count == 0) { // If no tokens are entered,
            continue; // continue to the next iteration of the loop
        }

        // Check for specific built-in commands:
        //1. "cd" 
        if (strcmp(args[0], "cd") == 0) { // If the first token is "cd"
            char *directory = args[1];
            
            if (directory == NULL) { // Default to home directory
                directory = getenv("HOME");
            } 
            if  (chdir(directory) == -1){ // Incase chdir fails
                perror("cd failed");
            }
            continue;
        }
        
        //2. "exit"
        if (strcmp(args[0], "exit") == 0) { // If the first token is "exit"
            break; // Exit the shell
        }
        
        //3. "&"
        int backgroundprocess = 0;
        
        if (arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0) { // If the last token is "&"
            args[arg_count - 1] = NULL; // Remove it from the args array
            backgroundprocess = 1; // Set background command flag to true
            arg_count--; // Decrements the arg counter
        }
        
        //4. "|"
        char **left_args = NULL;
        char **right_args = NULL;
        int pipe_index = -1;
        
        for (int i = 0; i < arg_count; i++) { // Loop through the tokens
            if (strcmp(args[i], "|") == 0) { // If "|"
                args[i] = NULL; // Set the token to NULL
                left_args = args; // Left side of the pipe
                right_args = &args[i + 1]; // Right side of the pipe
                pipe_index = i; // Store the index of the pipe
                break;
            }
        }
        
        if (pipe_index != -1) { // If pipe is found
            int pipefd[2]; // File descriptors for the pipe
            if (pipe(pipefd) == -1) { // If pipe fails
                perror("pipe failed"); // Print error message
                continue; // Continue to the next iteration of the loop
            }

            pid_t left_pid = fork(); // Fork the first child process
            if (left_pid == 0) { // Left Command/Child 
                close(pipefd[0]); // Close the read end of the pipe
                dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
                close(pipefd[1]); // Close the write end of the pipe
                execvp(left_args[0], left_args); // Execute the left command
                perror("left execvp failed"); // Print error message if execvp fails
                exit(1); // Exit the child process
            }

            pid_t right_pid = fork(); // Fork the second child process
            if (right_pid == 0) { // Right Command/Child 
                close(pipefd[1]); // Close the write end of the pipe
                dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to the read end of the pipe
                close(pipefd[0]); // Close the read end of the pipe
                execvp(right_args[0], right_args); // Execute the right command
                perror("right execvp failed"); // Print error message if execvp fails
                exit(1); // Exit the child process
            }

            close(pipefd[0]); // Close the read end of the pipe
            close(pipefd[1]); // Close the write end of the pipe

            waitpid(left_pid, NULL, 0); // Wait for the left child to finish
            waitpid(right_pid, NULL, 0); // Wait for the right child to finish
            continue; // Continue to the next iteration of the loop
            
        }

        //5. ">" or "<":
        char *outfilename = NULL;
        char *infilename = NULL;

        for (int i = 0; i < arg_count; i++) { // Loop through the tokens
            if (args[i] == NULL) {
                continue; // fixing timeout issue by continuing when a Null argument is encountered
            }
            if (strcmp(args[i], ">") == 0) { // If  ">" 
                outfilename = args[i+1]; // Get the filename that follows
                args[i] = NULL; // Set > the token to NULL
                args[i+1] = NULL; // Set the filename token to NULL 
            } else if (strcmp(args[i], "<") == 0) { // If "<"
                infilename = args[i+1]; // Get the filename that follows
                args[i] = NULL; // Set < the token to NULL
                args[i+1] = NULL; // Set the filename token to NULL
            }
        }


        
        // Fork:
        pid_t pid = fork(); // Call fork() and capture the return value
        if (pid == 0) { // Child process(exec)
            if (outfilename) { // If outfile is not NULL
                int fd = open(outfilename, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open file if needed
                if (fd == -1) { // If file could not be created
                    perror("failed to create file"); // Print error 
                    exit(1); // Terminate the child
                }
                dup2(fd, STDOUT_FILENO); // Redirect stdout to file
                close(fd); // Close the file descriptor
            }
            if (infilename) { // If filename is not NULL
                int fd = open(infilename, O_RDONLY); // Open file if needed
                if (fd == -1) { // If file could not be opened
                    perror("failed to open file"); // Print error
                    exit(1); // Terminate the child
                }
                dup2(fd, STDIN_FILENO); // Redirect stdin to file
                close(fd);
            }
            execvp(args[0], args); // If successful, execvp() will not return
            perror("execvp failed"); // Print error message if execvp fails
            exit(1); // Exit the child process
        } else if (pid > 0) { // Parent process(wait)
            if (!backgroundprocess) { // If not a background command
                waitpid(pid, NULL, 0); // Wait for the child 
            } else { // If background command
                printf("Background process PID: %d\n", pid); // Print prompt
            }
        } else { // Fork failed
            perror("fork failed"); // Print error message
            continue; // Continue to the next iteration of the loop
        }

        /* 
        Tests Used as the shell was being built:
        
        // Echo the input back to the user
        printf("You entered: %s\n", line);

        // loop to print the tokens line by line to varify that the parsing works
        for (int i = 0; i < arg_count; i++) {
             printf("Token %d: %s\n", i, args[i]);
        }
        */
    }
    return 0;
}
