
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
void launch_program_with_redirection(char *args[], int argsc, char lwd[]) {
    if (argsc > 0 && strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    int rc = fork();

    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc == 0) {

        for (int i = 0; i < argsc; ++i) {

            if (strcmp(args[i], "<") == 0) {
                char *file = args[i + 1];
                args[i] = NULL;

                char line[MAX_LINE] = "";

                for (int j = 0; j < i; j++) {
                    if (j > 0) strcat(line, " ");
                    strcat(line, args[j]);
                }
                if (is_subshell(line)) {
                    int fd = open(file, O_RDONLY);
                    if (fd == -1) {
                        perror("open");
                        _exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                    execute_command(line, lwd);
                    _exit(0);
                }
                else {
                    child_with_input_redirect(args, i, file);
                }
            }

            else if (strcmp(args[i], ">") == 0) {
                char *file = args[i + 1];
                args[i] = NULL;

                char line[MAX_LINE] = "";

                for (int j = 0; j < i; j++) {
                    if (j > 0) strcat(line, " ");
                    strcat(line, args[j]);
                }

                if (is_subshell(line)) {
                    int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                    if (fd == -1) {
                        perror("open");
                        _exit(1);
                    }

                    dup2(fd, STDOUT_FILENO);
                    close(fd);

                    execute_command(line, lwd);
                    _exit(0);
                }
                else {
                    child_with_output_redirect(args, i, file, false);
                }
            }

            else if (strcmp(args[i], ">>") == 0) {
                char *file = args[i + 1];
                args[i] = NULL;

                char line[MAX_LINE] = "";

                for (int j = 0; j < i; j++) {
                    if (j > 0) strcat(line, " ");
                    strcat(line, args[j]);
                }

                if (is_subshell(line)) {
                    int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);

                    if (fd == -1) {
                        perror("open");
                        _exit(1);
                    }

                    dup2(fd, STDOUT_FILENO);
                    close(fd);

                    execute_command(line, lwd);
                    _exit(0);
                }
                else {
                    child_with_output_redirect(args, i, file, true);
                }
            }
        }

        child(args, argsc);
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

bool is_pipe(char line[]) {
    int parentheses_count = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '(') {
            parentheses_count++;
        }
        else if (line[i] == ')') {
            parentheses_count--;
        }
        else if (line[i] == '|' && parentheses_count == 0) {
            return true;
        }
    }

    return false;
}

void tokenize_pipe(char *args[], int argsc, char *commands[][MAX_ARGS], int *commandnbr) {
    int i = 0;
    int command_index = 0;
    int argument_index = 0;
    int parentheses_count = 0;
    while (i < argsc) {
        for (int j = 0; args[i][j] != '\0'; j++) {
            if (args[i][j] == '(') {
                parentheses_count++;
            }
            if (args[i][j] == ')') {
                parentheses_count--;
            }
        }
        if (strcmp(args[i], "|") == 0 && parentheses_count == 0) {
            commands[command_index][argument_index] = NULL;
            command_index++;
            argument_index = 0;
            i++;
            continue;
        }
        commands[command_index][argument_index++] = args[i];
        i++;
    }
    commands[command_index][argument_index] = NULL;
    *commandnbr = command_index + 1;
}

void launch_pipe (char *commands[][MAX_ARGS], int commandnbr, char lwd[]){
    int fd[MAX_ARGS][2];
    
    for (int i = 0; i < commandnbr - 1; i++) {
    if (pipe(fd[i]) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }
    
}
    for (int i = 0; i < commandnbr; i++) {  
        int rc = fork();
        if (rc < 0){
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }
        else if(rc ==0) {
            if (i > 0) {
                dup2(fd[i-1][0], STDIN_FILENO);
            }       

            if (i < commandnbr - 1) {
                dup2(fd[i][1], STDOUT_FILENO);
            }
            for (int j = 0; j < commandnbr - 1; j++) {
                close(fd[j][0]);
                close(fd[j][1]);
            
            }
            char line[MAX_LINE] = "";

            for (int j = 0; commands[i][j] != NULL; j++) {
                if (j > 0) {
                    strcat(line, " ");
                }
                strcat(line, commands[i][j]);
            }

            execute_command(line, lwd);
            _exit(0);
            }
            
        
        }
    
    
    for (int j = 0; j < commandnbr - 1; j++) {
        close(fd[j][0]);
        close(fd[j][1]);
    }
    for (int j = 0; j < commandnbr; j++) {
            wait(NULL);
}
}


bool is_semicolon(char line[]) {
    int parentheses_count = 0;

    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '(') {
            parentheses_count++;
        }
        else if (line[i] == ')') {
            parentheses_count--;
        }
        else if (line[i] == ';' && parentheses_count == 0) {
            return true;
        }
    }

    return false;
}

void tokenize_semicolon(char *args[], int argsc,char *commands_semicolon[][MAX_ARGS], int *commandnbr_semicolon){ //outcount is a count of the overall index first count keeps the index where we see | and incount counts for stdin
    int i = 0;              // Position in the original args[]
    int command_index = 0;  // Which command we're filling
    int argument_index = 0; // Which argument in that command
    int parentheses_count = 0;
    while (i < argsc) {
        for (int j = 0; args[i][j] != '\0'; j++) {
            if (args[i][j] == '(') {
                parentheses_count++;
            }
            if (args[i][j] == ')') {
                parentheses_count--;
            }
        }

        if ((strcmp(args[i], ";") == 0) && (parentheses_count == 0)){
           commands_semicolon[command_index][argument_index] =NULL;
           command_index++;
           argument_index =0;
           i++;
           continue;
        }
        commands_semicolon[command_index][argument_index++]= args[i];
        i++;

    }
commands_semicolon[command_index][argument_index] = NULL;
*commandnbr_semicolon =command_index +1;
}

void launch_semicolon(char *commands_semicolon[][MAX_ARGS],int commandnbr_semicolon, char lwd[]) {

    for (int i = 0; i < commandnbr_semicolon; i++) {

        char line[MAX_LINE] = "";

        for (int j = 0; commands_semicolon[i][j] != NULL; j++) {
            if (j > 0) {
                strcat(line, " ");
            }

            strcat(line, commands_semicolon[i][j]);
        }

        execute_command(line, lwd);
    }
}

void execute_command(char line[], char lwd[]) {
    char *args[MAX_ARGS];
    int argsc;

    char *commands[MAX_ARGS][MAX_ARGS];
    int commandnbr;

    char *commands_semicolon[MAX_ARGS][MAX_ARGS];
    int commandnbr_semicolon;
    if (is_subshell(line)) {
        char subshell_line[MAX_LINE];

        tokenize_subshell(line, subshell_line);
        launch_subshell(subshell_line, lwd);
    }
    else if (is_semicolon(line)) {
        parse_command(line, args, &argsc);
        tokenize_semicolon(args, argsc, commands_semicolon, &commandnbr_semicolon);
        launch_semicolon(commands_semicolon, commandnbr_semicolon, lwd);
    }
    
    else if (is_cd(line)) {
        parse_command(line, args, &argsc);
        run_cd(args, argsc, lwd);
    }
    else if (is_pipe(line)) {
        parse_command(line, args, &argsc);
        tokenize_pipe(args, argsc, commands, &commandnbr);
        launch_pipe(commands, commandnbr, lwd);
    }
    else if (command_with_redirection(line)) {
        parse_command(line, args, &argsc);
        launch_program_with_redirection(args, argsc, lwd);
        reap();
    }
    else {
        parse_command(line, args, &argsc);
        if (argsc == 0) {
            return;
        }
        launch_program(args, argsc);
        reap();
    }
}

bool is_subshell(char line[]) {
    int parentheses_count = 0;
    int start = 0;

    while (line[start] == ' ') {
        start++;
    }
    if (line[start] != '(') {
        return false;
    }

    for (int i = start; line[i] != '\0'; i++) {
        if (line[i] == '(') {
            parentheses_count++;
        }

        if (line[i] == ')') {
            parentheses_count--;
        }
        if (parentheses_count < 0) {
            return false;
        }
        if (parentheses_count == 0) {
            for (int j = i + 1; line[j] != '\0'; j++) {
                if (line[j] != ' ') {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}


void tokenize_subshell(char line[], char subshell_line[]){
    int parentheses_count = 0;
    int j = 0;
    

    for (int i=0;line[i]!='\0';i++){
        if ((line[i] =='(')){
            parentheses_count++;
            if (parentheses_count==1){
                continue;
            }
            
        }
        if((line[i] ==')')){
            parentheses_count--;
            if(parentheses_count==0){
                break;
            }
        }
        if (parentheses_count>0){
            subshell_line[j]=line[i];
            j++;
        }
        

    }
    subshell_line[j]='\0';
}
void launch_subshell(char subshell_line[], char lwd[]){
    int rc = fork();

    if (rc<0){
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (rc ==0) {
        execute_command(subshell_line, lwd);
        _exit(0);
    } 
    else{
        wait(NULL);
    }
}