#include "s3.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[], char lwd[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        perror("fgets failed");
        exit(1);
    }
    ///Remove newline (enter)
    line[strlen(line) - 1] = '\0';
}

void parse_command(char line[], char *args[], int *argsc)
{
    ///Implements simple tokenization (space delimited)
    ///Note: strtok puts '\0' (null) characters within the existing storage, 
    ///to split it into logical cstrings.
    ///There is no dynamic allocation.

    ///See the man page of strtok(...)
    char *token = strtok(line, " ");
    *argsc = 0;
    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}

///Launch related functions
void child(char *args[], int argsc)
{
    ///Implement this function:

    ///Use execvp to load the binary 
    ///of the command specified in args[ARG_PROGNAME].
    ///For reference, see the code in lecture 3.
    execvp(args[ARG_PROGNAME], args);
    perror("execvp failed");
    exit(1);
}

void launch_program(char *args[], int argsc)
{
    ///Implement this function:

    ///fork() a child process.
    ///In the child part of the code,
    ///call child(args, argv)
    ///For reference, see the code in lecture 2.

    ///Handle the 'exit' command;
    ///so that the shell, not the child process,
    ///exits.

    if (argsc == 0)
    {
        return;
    }
    if (strcmp(args[0], "exit")==0){
        exit(0);
    }
    int rc = fork();
    
    if (rc == 0){
        child(args, argsc);
    }
    if (rc<0){
        perror("rc<0");
    }
    

}

int command_with_redirection(char line[]){
    for (int i = 0; i < strlen(line); i++){
        if (line[i]=='<'){
            return(1);
        }
        if (line[i]=='>'){
            return(1);
        }

    }
    return(0);

}
void launch_program_with_redirection(char *args[], int argsc){
    int rc = fork();
        if (rc == 0){

            for (int i = 0; i < argsc; i++){
                char *filename = args[i + 1];
                if (strcmp(args[i],"<")==0){
                    if (filename == NULL) {
                        fprintf(stderr, "Missing filename\n");
                        exit(1);
                    }
                    
                    args[i] = NULL;
                    int fd = open(filename, O_RDONLY, 0644);
                    if (fd == -1) {
                        perror("open failed");
                    exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
                else if (strcmp(args[i],">")==0){
                    char *filename = args[i + 1];
                    if (filename == NULL) {
                        fprintf(stderr, "Missing filename\n");
                        exit(1);
                    }
                    
                    args[i] = NULL;
                    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("open failed");
                    exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                else if (strcmp(args[i],">>")==0){
                    char *filename = args[i + 1];
                    if (filename == NULL) {
                        fprintf(stderr, "Missing filename\n");
                        exit(1);
                    }
                    
                    args[i] = NULL;
                    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    if (fd == -1) {
                        perror("open failed");
                    exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
            
            }
            child(args, argsc);
        }
        if (rc<0){
            perror("rc<0");
        }
        
}
void init_lwd(char lwd[]){
    getcwd(lwd, MAX_PROMPT_LEN-6);
};

int is_cd(char line[]){
    int i =0;
    while (line[i]==' '){
        i++;
    }
    if (line[i]=='c'){
        if (line[i+1]=='d'){
            if (line[i+2]==' '|| line[i+2] == '\0'){
                return 1;
            }
        }

    }
    return 0;

}

void run_cd(char *args[], int argsc, char lwd[]){
    char *home = getenv("HOME");
    char old_dir[MAX_PROMPT_LEN - 6];

    if (getcwd(old_dir, sizeof(old_dir)) == NULL) {
        fprintf(stderr, "getcwd failed\n");
        return;
    }

    if (args[1] == NULL){  // If only cd then go to home directory
        if (home == NULL) {
            fprintf(stderr, "HOME not found\n");
            return;
        }

        if (chdir(home) == 0) {
            strcpy(lwd, old_dir);
        } else {
            fprintf(stderr, "chdir failed\n");
        }
    }
    else if (strcmp(args[1], "-") == 0){  // Go back to previous directory
        if (chdir(lwd) == 0) {
            strcpy(lwd, old_dir);
        } else {
            fprintf(stderr, "chdir failed\n");
        }
    }
    else{
        if (chdir(args[1]) == 0) {
            strcpy(lwd, old_dir);
        } else {
            fprintf(stderr, "chdir failed\n");
        }
    }
}