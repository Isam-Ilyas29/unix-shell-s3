
#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory
    char lwd[MAX_PROMPT_LEN-6];

    init_lwd(lwd);

    ///Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    char *commands[MAX_ARGS][MAX_ARGS];
    int commandnbr;

    while (1) {
        read_command_line(line, lwd); ///Notice the additional parameter (required for prompt construction)

        if (is_cd(line)) {
            parse_command(line, args, &argsc);
            run_cd(args, argsc, lwd);
        }
        
        else if (command_with_redirection(line)) {
            ///Command with redirection
            parse_command(line, args, &argsc);
            launch_program_with_redirection(args, argsc);
            reap();
        }
        else if (is_pipe(line)) {
            parse_command(line, args, &argsc);
            tokenize_pipe(args, argsc, commands, &commandnbr);
            launch_pipe(commands, commandnbr);
        }
        
        else {
            ///Basic command
            parse_command(line, args, &argsc);
            launch_program(args, argsc);
            reap();
        }
    }

    return 0;
    
}
