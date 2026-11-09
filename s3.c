#include "s3.h"

/// Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    char cwd[MAX_PROMPT_LEN-7];
    getcwd(cwd, MAX_PROMPT_LEN-6);
    snprintf(shell_prompt, MAX_PROMPT_LEN, "[%s s3]$ ", cwd);
}

/// Prints a shell prompt and reads input from the user
void read_command_line(char line[], char lwd[])
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
    perror(args[0]); // Unrecognised command
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
    perror(args[0]); // Unrecognised command
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
    perror(args[0]); // Unrecognised command
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

void init_lwd(char lwd[])
{
    getcwd(lwd, MAX_PROMPT_LEN-6);
}

bool is_cd(char line[])
{
    int i = 0;
    while (line[i] == ' ') i++;
    if (line[i] == 'c' && line[i+1] == 'd' && (line[i+2] == ' ' || line[i+2] == '\0')) return true;
    return false;
}

void run_cd(char *args[], int argsc, char lwd[])
{
    char* target;

    if (argsc == 1)
    {
        target = getenv("HOME");
    }
    else if (argsc == 2 && strcmp(args[1], "-") == 0)
    {
        target = lwd;
    }
    else if (argsc == 2)
    {
        target = args[1];
    }
    else
    {
        fprintf(stderr, "cd: too many arguments\n");
        return;
    }

    char prev[MAX_PROMPT_LEN-6];
    getcwd(prev, MAX_PROMPT_LEN-6); // TODO: why is this line necessary?

    if (chdir(target) == -1)
    {
        perror("cd");
    }
    else
    {
        strcpy(lwd, prev);
    }
}