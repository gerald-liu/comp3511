#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
//#include <sys/wait.h> // In C99 and later 

// Assume that each command line has at most 256 characters (including NULL)
#define MAX_CMDLINE_LEN 256

// Assume that we have at most 16 pipe segments
#define MAX_PIPE_SEGMENTS 16

// Assume that each segment has at most 256 characters (including NULL)
#define MAX_SEGMENT_LENGTH 256

/*
  Function  Prototypes
 */
void show_prompt();
int get_cmd_line(char *cmdline);
void process_cmd(char *cmdline);
void **tokenize(char **argv, char *line, int *numTokens, char *token);


/* The main function implementation */
int main()
{
    char cmdline[MAX_CMDLINE_LEN];
    printf("The shell program (pid=%d) starts\n", getpid());
    while (1)
    {
        show_prompt();
        if (get_cmd_line(cmdline) == -1)
            continue; /* empty line handling */

        process_cmd(cmdline);
    }
    return 0;
}

/* 
    Implementation of process_cmd

    TODO: Clearly explain how you implement process_cmd in point form. For example:

    Step 1: separate the pipe segments and store them in an array.
    Step 2: separate each command segment into a vector of arguments and store the commands in an array.
    Step 3: if the first command is the built-in exit command, exit the program.
    Step 4: loop through the commands and execute them one by one.
        Step 4.1: create the pipe.
        Step 4.2: fork the current process.
        Step 4.3: if the forking fails, prompt error and exit.
        Step 4.4: in the child process:
            Step 4.4.1: direct the output of the previous command to the current input.
            Step 4.4.2: if the child has a successor, make the current output as the pipe input.
            Step 4.4.3: close the read end of the pipe.
            Step 4.4.5: execute the command, prompt if an error occurs.
            Step 4.4.6: exit the child process.
        Step 4.5: in the parent process:
            Step 4.5.1: wait for the child process to finish.
            Step 4.5.2: close the write end of the pipe.
            Step 4.5.3: store the output of the child.
            Step 4.5.4: move on to the next command.
 */
void process_cmd(char *cmdline)
{
    // Step 1: separate the pipe segments
    char* pipe_segments[MAX_PIPE_SEGMENTS];
    int num_segments = 0;
    char pipe_delim[2] = "|";
    tokenize(pipe_segments, cmdline, &num_segments, pipe_delim);

    // Step 2: separate each segment into a vector of arguments
    int MAX_ARGC = 10; // there are at most 9 positional parameters, so argc <= 10
    char* cmd[MAX_PIPE_SEGMENTS][MAX_ARGC + 1]; // tokenize() sets argv[argc+1] to NULL
    char arg_delim[2] = " ";

    int i; // ‘for’ loop initial declarations are only allowed in C99 mode
    for (i = 0; i < num_segments; ++i) {
        int argc = 0; // not used
        tokenize(cmd[i], pipe_segments[i], &argc, arg_delim); // *cmd = argv
    }

    // Step 3: built-in exit command
    if (strcmp("exit", cmd[0][0]) == 0) {
        printf("The shell program (pid=%d) terminates\n", getpid());
        exit(0); // EXIT_SUCCESS
    }

    // Step 4: loop through the commands and execute them one by one
    int fd[2];
    pid_t pid;
    int fd_tmp = 0;

    for (i = 0; i < num_segments;) {
        pipe(fd); // create the pipe
        pid = fork(); // fork the current process

        if (pid == -1) { // if the forking fails, prompt error and exit
            perror("\nError in forking.");
            exit(1); // EXIT_FAILURE
        }

        else if (pid == 0) { // Child process
            dup2(fd_tmp, 0); // direct the output of the previous command to the current input

            if (i < num_segments - 1) // if the child has a successor
                dup2(fd[1], 1); // make the current output as the pipe input
            
            close(fd[0]); // close the read end of the pipe

            if (execvp(cmd[i][0], cmd[i]) == -1) { // execute the command
                perror("\nError in command"); // prompt if an error occurs
                _exit(1); // exit the child process, EXIT_FAILURE
            }
            else
                _exit(0);  // exit the child process, EXIT_SUCCESS
        }
        
        else { // Parent process
            wait(0); // wait for the child process to finish

            close(fd[1]); // close the write end of the pipe
            fd_tmp = fd[0]; // store the output of the child
            
            ++i; // move on to the next command
        }
    }
}


void show_prompt()
{
    printf("$> ");
}

// check whether the command is empty (-1) or not (0)
int get_cmd_line(char *cmdline)
{
    int i;
    int n;
    if (!fgets(cmdline, MAX_CMDLINE_LEN, stdin))
        return -1;
    // Ignore the newline character
    n = strlen(cmdline);
    cmdline[--n] = '\0';
    i = 0;
    while (i < n && cmdline[i] == ' ') {
        ++i;
    }
    if (i == n) {
        // Empty command
        return -1;
    }
    return 0;
}

// get argv and numTokens
void **tokenize(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    argv[argc++] = NULL;
    *numTokens = argc - 1;
}
