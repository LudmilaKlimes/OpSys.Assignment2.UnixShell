# OpSys.Assignment2.UnixShell

## Quick Overview
***A simple Unix-like shell implemented in C.***  

Includes:
-  command execution
-  input/output redirection
-  pipelines
-  background process handling
-  basic built-in commands

This project was used to gain hands-on experience with operating system concepts such as process creation, inter-process communication, file descriptor manipulation, and signal handling. 

## Features
1. **Command Execution**  
Executes standard Unix commands using `fork()` and `execvp()`.
Parses user input into tokens for execution.

2. **Input/Output Redirection**  
Input redirection (`<`) redirects standard input from a file.
Output redirection (`>`) redirects standard output to a file (creates/overwrites).

3. **Pipelines**  
Allows the use of a pipe (`|`) to pass the output of one command to the input of another using `pipe()` and `dup2()`.

4. **Background Processes**  
Runs commands in the background using `&`, allowing the shell to continue accepting new commands without waiting.
Displays the PID of background processes.

5. **Built-in Commands**  
`cd` - Changes the current working directory (defaults to `HOME` if no directory is provided).
`exit` - Terminates the shell.

6. **Signal Handling**  
Handles `SIGCHLD` to clean up terminated background processes.
Prevents zombie processes using `waitpid()` with `WNOHANG`.

## How It Works
1. The shell displays a prompt (`mysh>`) and waits for user input.
2. Reads input with `fgets()`.
3. Input is parsed into tokens using `strtok()`.
4. Checks for:
    * Built-in commands (`cd`, `exit`)
    * Background execution (`&`)
    * Pipes (`|`)
    * Input/Output redirection (`<`, `>`)
5. Executes commands by:
    * Creating child processes with `fork()`
    * Redirecting file descriptors with `dup2()`
    * Running commands with `execvp()`
6. The parent process:
    * Waits for foreground processes
    * Continues immediately for background processes

## Build & Run
`gcc -o mysh mysh.c`
`./mysh`
