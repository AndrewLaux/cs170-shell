//A simple shell program in c++
//Written by: ANDREW LAUX
//CS170 UCSB 2019
#include <stdio.h>
#include <exception>
#include "task_manager.h"


//*************************************************************************
// -- main() --
// Launches the shell program. Takes valid commands and executes them until
// the user indicates EOF using ctrl+D. Returns 0 upon successful completi-
// on. Given an invalid command, main should simply as for another. 
int main(int argc, char** argv) {

    //Declare input variable:
    char input[512];
    bool standby = true;

    //Instance task_manager:
    task_manager manager;

    //Loop input prompt while not EOF:
    while (standby) {

        //Print Prompt:
        printf("shell: ");
        
        //Get shell command:
        if (fgets(input, 512, stdin) != NULL) {

            //Try to preform command
            try {
                manager.update_command(input);
                manager.run_input();
            }
            catch (std::exception& e) { printf("ERROR: %s", e.what()); }   
        }
        
        //EOF. End program:
        else standby = false;
        
    }

    //Successful completion.
    return 0;
}


