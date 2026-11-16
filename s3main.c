
#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory
    char lwd[MAX_PROMPT_LEN-6];

    init_lwd(lwd);


    while (1) {
    read_command_line(line, lwd);
    execute_command(line, lwd);
    }

    return 0;
    
}
