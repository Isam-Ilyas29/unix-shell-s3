#include "s3.h"

/// Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

/// Prints a shell prompt and reads input from the user
void read_command_line(char line[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    /// See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        perror("fgets failed");
        exit(1);
    }
    /// Remove newline (enter)
    line[strlen(line) - 1] = '\0';
}

void parse_command(char line[], char *args[], int *argsc)
{
    /// Implements simple tokenization (space delimited)
    /// Note: strtok puts '\0' (null) characters within the existing storage,
    /// to split it into logical cstrings.
    /// There is no dynamic allocation.

    /// See the man page of strtok(...)
    char *token = strtok(line, " ");
    *argsc = 0;
    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }

    args[*argsc] = NULL; /// args must be null terminated
}

/// Launch related functions
void child(char *args[], int argsc)
{
    /// Implement this function:

    /// Use execvp to load the binary
    /// of the command specified in args[ARG_PROGNAME].
    /// For reference, see the code in lecture 3.
    execvp(args[0], args);
    exit(1);
}

void child_with_input_redirect(char *args[], int argsc, char file[])
{
    int fd = open(file, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }
    dup2(fd, STDIN_FILENO);
    close(fd);
    execvp(args[0], args);
    exit(1);
}
void child_with_output_redirect(char *args[], int argsc, char file[], bool append)
{   
    int fd;
    if (append)
    {
        fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644); // chmod 0664
    }
    else
    {
        fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644); // chmod 0664
    }
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
    execvp(args[0], args);
    exit(1);
}

void launch_program(char *args[], int argsc)
{
    /// Implement this function:

    /// fork() a child process.
    /// In the child part of the code,
    /// call child(args, argv)
    /// For reference, see the code in lecture 2.

    /// Handle the 'exit' command;
    /// so that the shell, not the child process,
    /// exits.

    if (argsc > 0 && strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }

    int rc = fork();
    if (rc < 0)
    {
        // Error
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0)
    {
        // Child
        child(args, argsc);
    }
    else
    {
        // Parent
    }
}

///
bool command_with_redirection(char line[])
{
    for (int i = 0; i < strlen(line); ++i)
    {
        if (line[i] == '>' || line[i] == '<')
        {
            return 1;
        }
    }
    return 0;
}

///
void launch_program_with_redirection(char *args[], int argsc)
{
    if (argsc > 0 && strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }

    int rc = fork();
    if (rc < 0)
    {
        // Error
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0)
    {
        // Child
        // store first and second half then pass
        for (int i = 0; i < argsc; ++i)
        {
            if (strcmp(args[i], "<") == 0)
            {
                char *file = args[i + 1];
                args[i] = NULL;
                child_with_input_redirect(args, i, file);
            }
            else if (strcmp(args[i], ">") == 0)
            {   
                char *file = args[i + 1];
                args[i] = NULL;
                child_with_output_redirect(args, i, file, false);
            }
            else if (strcmp(args[i], ">>") == 0)
            {
                char *file = args[i + 1];
                args[i] = NULL;
                child_with_output_redirect(args, i, file, true);
            }
        }
        child(args, argsc);
    }
    else
    {
        // Parent
    }
}