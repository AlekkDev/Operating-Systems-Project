// isnt working without this 
#define _POSIX_C_SOURCE 200112L 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>

// if systems do not define HOST_NAME_MAX this fixes it 
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256 
#endif
// def max input size and arguments
#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64  

// Function declarations for better organization
void execute_command(char *args[], int background, char *output_file);
void handle_quit();
void print_globalusage();

int main() {
    char input[MAX_INPUT_SIZE];   //  store user input
    char *args[MAX_ARGS];         // Array to hold parsed command arguments
    char *token;                  // Temporary pointer for tokenized input
    int background;               // Flag to indicate background execution
    char *output_file = NULL;     // File for output redirection (if any)

    // Main shell loop (runs until quit is called)
    while (1) {
        // reset background flag and output file for each new command
        background = 0;
        output_file = NULL;

        // fetch hostname and user for the shell prompt
        char hostname[HOST_NAME_MAX];
        gethostname(hostname, HOST_NAME_MAX); // Get hostname
        printf("%s@%s> ", getlogin(), hostname); // Display prompt as user@hostname>

        // Get user input 
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            break; // Exit on NULL
        }

        // remove annoying newline character from the input
        input[strcspn(input, "\n")] = 0;

        // Tokenize the input string into individual arguments
        int i = 0; // Counter for args array
        token = strtok(input, " "); // Split string by spaces
        while (token != NULL) {
            if (strcmp(token, "&") == 0) {  //
                background = 1;
            } else if (strcmp(token, ">") == 0) { // 
                token = strtok(NULL, " "); // Get next token (filename)
                output_file = token;       // Store ouput filename 
            } else {
                args[i++] = token;         // Add token to args array
            }
            token = strtok(NULL, " "); // Move to the next token
        }
        args[i] = NULL; // Null-terminate the args array for execvp

        // Handle internal commands or pass to external executables
        if (args[0] == NULL) {
            continue; // Ignore if empty input
        } else if (strcmp(args[0], "globalusage") == 0) {
            print_globalusage(); // Show shell version + authors info
        } else if (strcmp(args[0], "quit") == 0) {
            handle_quit(); // Exit shell and handle any running processes
            break;         // Exit loop after handle_quit
        } else if (strcmp(args[0], "exec") == 0) {
            if (i < 2) { // Ensure a program is specified after "exec"
                fprintf(stderr, "Usage: exec <program_to_execute>\n");
            } else {
                execute_command(&args[1], background, output_file); // Execute external command
            }
        } else {
            fprintf(stderr, "Unknown command: %s\n", args[0]); // Error for unrecognized commands
        }
    }

    return 0; // Exit the shell 
}

// Execute a command, with optional background execution and output redirection
void execute_command(char *args[], int background, char *output_file) {
    pid_t pid = fork(); // Create a new process

    if (pid < 0) { // Error handling for fork failure
        perror("fork");
        return;
    } else if (pid == 0) { // Child process
        if (output_file != NULL) { // If output redirection is requested
            int fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0) { // Handle file opening errors
                perror("open");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO); // Redirect stdout to the file
            close(fd); // Close fd
        }

        execvp(args[0], args); // Execute the command
        perror("execvp"); // Print error if execvp fails
        exit(1); // Exit child if error
    } else { // Parent process
        if (!background) { // 
            int status;
            waitpid(pid, &status, 0); // Wait for child process to finish
            printf("Process %d finished\n", pid); // Print PID of completed process
        } else { // Background execution
            printf("Process %d running in background\n", pid);
        }
    }
}

// Handle quit command by checking for any running processes
void handle_quit() {
    int status;
    pid_t pid;

    // Check if any running processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Process %d is still running\n", pid);
    }

    if (pid == 0) { // If processes still running
        char response;
        printf("The following processes are running, are you sure you want to quit? [Y/N]: ");
        scanf(" %c", &response);

        if (response == 'Y' || response == 'y') {
            kill(-getpgrp(), SIGTERM); // terminate all processes in the group
            printf("Shell terminated.\n");
        } else {
            printf("Quit canceled.\n");
        }
    } else { // no processes are running
        printf("No processes running. Shell terminated.\n");
    }
}

// Print global usage information for the shell
void print_globalusage() {
    printf("IMCSH Version 1.1 created by Alek and Erwin\n"); // version that is executed and authors printed
}
