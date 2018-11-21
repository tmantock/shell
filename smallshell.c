//
//  smallshell.c
//  Shell
//
//  Created by Tevin Mantock on 5/27/18.
//  Copyright © 2018 Tevin Mantock. All rights reserved.
//

#include "smallshell.h"

int ForegroundOnly = 0;
char *builtin_str[] = {"cd", "status", "exit"};

int (*builtin_func[])(char **, int, int) = {
    &shell_cd,
    &shell_status,
    &shell_exit};

// Function gets the number of built in functions for the shell
int shell_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

// Function changes the directory the shell is in
int shell_cd(char **args, int size, int status)
{
    // Go to home directory if the args is empty
    if (args[1] == NULL)
    {
        chdir(getenv("HOME"));
    }
    else
    {
        // Change to the directory chosen
        if (chdir(args[1]) != 0)
        {
            perror("Shell");
        }
    }

    return 1;
}

// Function shows the status of the last foreground prcoess
int shell_status(char **args, int size, int status)
{
    // Check for exit status
    if (WIFEXITED(status))
    {
        printf("Shell: Last Foreground Process exited with an exit status of %d\n", status);
    }
    // Check signal status
    else if (WIFSIGNALED(status))
    {
        printf("Shell: Last Foreground Process was terminated by signal %d\n", status);
    }

    return 1;
}

// Function causes the shell to exit
int shell_exit(char **args, int size, int status)
{
    return 0;
}

// Function sets up a loop that runs until the user calls the exit command
void shell_loop(void)
{
    char *line;
    List *args;
    Processes *proc = create_processes(); // Data Structure to track background processes
    int status = 0;

    do
    {
        write(STDOUT_FILENO, ": ", 2);
        line = shell_read_line();      // get input
        args = shell_split_line(line); // Parse input

        status = shell_execute(args, status, proc); // Execute the args

        check_background_process(proc); // Check for background processes
        // Free memory
        free(line);
        destroyList(args);
        line = NULL;
        args = NULL;
    } while (status);

    destroy_proccess(proc);
}

// Function reads user input from stdin
char *shell_read_line(void)
{
    int bufsize = SHELL_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer)
    {
        fprintf(stderr, "Shell Allocation Error\n");
        exit(EXIT_FAILURE);
    }
    // Get input char by char until newline or EOF
    do
    {
        c = getchar();

        buffer[position++] = c;

        if (position >= bufsize)
        {
            bufsize += SHELL_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);

            if (!buffer)
            {
                fprintf(stderr, "Shell Allocation Error\n");
                exit(EXIT_FAILURE);
            }
        }
    } while (c != EOF && c != '\n');

    buffer[position] = '\0';

    return buffer;
}

// Function calls the parse input function to get the args list
List *shell_split_line(char *line)
{
    List *tokens = parse_input(line);

    return tokens;
}

// Function launches programs that are not implemented by the shell
int shell_launch(List *args, Processes *proc)
{
    pid_t pid;
    int status;
    int background = 0;
    InputNode *pipes[3]; // Hold args that change redirection (input, output)
    // Signal handlers
    struct sigaction sigint_action = {0};
    struct sigaction sigstop_action = {0};
    // Set the signal handlers
    sigint_action.sa_handler = sigint_handler;
    sigstop_action.sa_handler = sigstp_handler;

    sigfillset(&(sigint_action.sa_mask));
    sigfillset(&(sigstop_action.sa_mask));

    sigint_action.sa_flags = 0;
    sigstop_action.sa_flags = 0;

    sigaction(SIGINT, &sigint_action, NULL);
    sigaction(SIGTSTP, &sigstop_action, NULL);

    memset(pipes, 0, 3 * sizeof(InputNode *));
    // Check if there's any redirection or background processes
    for (int i = 0; i < args->count; i++)
    {
        if (args->container[i]->ops == '<' && i + 1 < args->count)
        { // Input
            pipes[0] = args->container[i + 1];
        }
        else if (args->container[i]->ops == '>' && i + 1 < args->count)
        { // Output
            pipes[1] = args->container[i + 1];
        }
        else if (args->container[i]->ops == '&' && !ForegroundOnly)
        {
            background = 1;
        }
    }
    // Fork to create a new prcoess
    pid = fork();
    if (pid == 0)
    { // Child Process
        // Set signal handlers to default
        sigint_action.sa_handler = SIG_DFL;
        sigstop_action.sa_handler = SIG_DFL;
        // Get the first command
        InputNode *nodeOne = listNextNode(args);
        // Check if there's input redirection from the user
        if (pipes[0] != NULL)
        {
            int input = open(pipes[0]->line[0], O_RDONLY, 06444);

            if (input == -1)
            {
                printf("%s: Unable to open input file\n", pipes[0]->line[0]);
                exit(1);
            }
            // Switch from STDIN to input file
            dup2(input, 0);
        }
        // Check if there's output redirection provided by the user
        if (pipes[1] != NULL)
        {
            int output = open(pipes[1]->line[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (output == -1)
            {
                printf("%s: Unable to open output file\n", pipes[1]->line[0]);
                exit(1);
            }
            // Switch from STDOUT to output file
            dup2(output, 1);
        }
        // Check if the process is to run in the background and if the input needs to point to null
        if (pipes[0] == NULL && background)
        {
            int input = open("/dev/null", O_RDONLY, 06444);

            if (input == -1)
            {
                printf("%s: Unable to open input file\n", "dev/null");
                exit(1);
            }
            // Change input from STDIN to null
            dup2(input, 0);
        }
        // Check if the process is to run in the background and if the output needs to point to null
        if (pipes[1] == NULL && background)
        {
            int output = open("/dev/null", O_RDONLY, 06444);

            if (output == -1)
            {
                printf("%s: Unable to open output file\n", "dev/null");
                exit(1);
            }
            // Change output from STDOUT to null
            dup2(output, 1);
        }
        // Kill this process code and have the program run with this process id
        if (execvp(nodeOne->line[0], nodeOne->line) == -1)
        {
            printf("%s: no such file or directory", nodeOne->line[0]);
        }
        // If code reaches here there was a problem
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    { // Error in fork process
        perror("Shell: Error starting child process through fork");
    }
    else
    { // Parent process
        if (background)
        {                           // If there was a background process
            add_process(proc, pid); // Add the process to the background list
            printf("Background PID is %d\n", pid);
            return 1; // Exit early to avoid waiting
        }
        do
        { // Wait for the child process to terminate
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

// Function searches the list of built in function to determine if there's local execution
// before calling the fork process to call external programs
int shell_execute(List *args, int status, Processes *proc)
{
    // Check if there are rgs
    if (args == NULL)
    {
        return 1; // Empty command
    }
    // Search the built-in list for the program
    for (int i = 0; i < shell_num_builtins(); i++)
    {
        if (strcmp(args->container[0]->line[0], builtin_str[i]) == 0)
        {
            // Call the function if it is found
            return (*builtin_func[i])(args->container[0]->line, args->container[0]->size, status);
        }
    }
    // Call the fork function
    return shell_launch(args, proc);
}

// Function checks the list of background prcoesses for any processes that have exited
void check_background_process(Processes *proc)
{
    int status = 0;
    // Look through the list
    for (int i = 0; i < proc->count; i++)
    {
        // Wait for the process with no hang (no block)
        pid_t pid = waitpid(proc->process[i], &status, WNOHANG);
        // Check for exit
        if (WIFEXITED(status) && pid > 0)
        {
            // remove pid if exited
            remove_process(proc, pid);
            printf("Background PID: %d is done: exit value %d\n", pid, status);
        }
        // Check for signal terminate
        else if (WIFSIGNALED(status) && pid > 0)
        {
            // remove process if terminated
            remove_process(proc, pid);
            printf("Background PID: %d is done: terminated by signal %d\n", pid, status);
        }
    }
}

// Function handles SIGINT signal and writes the signal number that terminated
// the foreground program
void sigint_handler(int sn)
{
    char str[60];
    int count = 0;

    memset(str, '\0', 60 * sizeof(char));

    sprintf(str, "Terminated by signal %d\n", sn);

    for (int i = 0; i < 60; i++)
    {
        if (str[i] != '\0')
        {
            count++;
        }
        else
        {
            break;
        }
    }

    write(STDOUT_FILENO, str, count);
    fflush(stdout);
}

// Function handles the SIGSTP signal
void sigstp_handler(int sn)
{
    // Toggle the foreground mode
    ForegroundOnly = !ForegroundOnly;
    // Print the appropiate message for the foreground mode
    if (ForegroundOnly)
    {
        write(STDOUT_FILENO, "Entering foreground-only mode (& is now ignored)\n", 49);
    }
    else
    {
        write(STDERR_FILENO, "Exiting foreground-only mode\n", 29);
    }
    fflush(stdout);
}

// Function creates a process list and initializes it
Processes *create_processes()
{
    Processes *proc = (Processes *)malloc(sizeof(Processes));
    proc->size = 10;
    proc->process = (pid_t *)malloc(sizeof(pid_t) * proc->size);
    proc->count = 0;

    return proc;
}

// Function adds a pid to the list
void add_process(Processes *p, pid_t proc)
{
    // Add the pid
    p->process[p->count++] = proc;
    // Check if resize is necessary
    if (p->count == p->size)
    {
        p->size *= 2;
        p->process = realloc(p->process, sizeof(pid_t) * p->size);
    }
}

// Function removes a pid from the list
void remove_process(Processes *p, pid_t pid)
{
    int index = -1;

    for (int i = 0; i < p->count; i++)
    {
        if (p->process[i] == pid)
        {
            index = i;
            break;
        }
    }

    for (int i = index + 1; i < p->count; i++)
    {
        p->process[i - 1] = p->process[i];
    }

    p->count--;
}

// Function frees a Processes list
void destroy_proccess(Processes *p)
{
    free(p->process);
    free(p);
}
