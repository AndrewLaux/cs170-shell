#include "task_manager.h"


//*************************************************************************
// -- Function: Default constructor --
// Creates a singleton task manager. Initializes correct PATH location for
// executables.
task_manager::task_manager() {
    cmd_list = std::vector<std::string>();
    task_list = std::vector<task>();
}

void task_manager::update_command(char *const cmd){

    //TODO: Check that taskmanager is ready for a new command input.

    //Take new cmd line and parse into whitespace seperated commands:
    cmd_list = std::vector<std::string>();
    std::string cmd_line = std::string(cmd);
    std::stringstream strm(cmd_line);
    std::string token;
    while (strm >> token) {
        this->cmd_list.push_back(token);
    }

    //Get locations of pipes if they exist:
    int last_pipe = -1;
    int first_pipe = -1;
    for (size_t i = 0; i < cmd_list.size(); i++) {
        if (cmd_list[i] == "|") {
            if (first_pipe == -1) {
                first_pipe = i;
                last_pipe = i;
            } else last_pipe = i;
        }
    }

    //Check for incorrect redirection in relation to pipes:
    if (first_pipe > -1) {
        for (size_t i = 0; i < cmd_list.size(); i++) {
            if (cmd_list[i] == ">" && int(i) < last_pipe) throw std::runtime_error("Bad output redirection\n");
            if (cmd_list[i] == "<" && int(i) > first_pipe) throw std::runtime_error("Bad input redirection\n");
        }
    }

    //Parse command tokens into tasks:
    size_t checkpoint = 0;
    while (checkpoint < cmd_list.size()) {
        task temp;
        temp.executable  = cmd_list[checkpoint];
        checkpoint++;

        //Remaining tokens are args for task:
        while (checkpoint < cmd_list.size()) {

            //Found an arg:
            if (cmd_list[checkpoint] != "|") {
                temp.args.push_back(cmd_list[checkpoint]);
                checkpoint++;
            }

            //Hit a pipe, no more args:
            else {
                checkpoint++;
                break;
            }   
        }

        //Check if task is schedueled for background execution:
        if (cmd_list.back() == "&") temp.background = true;

        //Look for input redirection and add filename to task:
        int in_index = -1;
        for (size_t i = 0; i < temp.args.size(); i++) {
            if (temp.args[i] == "<") {
                in_index = int(i);
                temp.infile = temp.args[i + 1];
                break;
            }

        }

        //Consume redirection tokens;
        if (in_index > -1) temp.args.erase(temp.args.begin() + in_index, temp.args.begin() + in_index + 2);
        
        //Look for output redirection and add filename to task:
        int out_index = -1;
        for (size_t i = 0; i < temp.args.size(); i++) {
            if (temp.args[i] == ">") {
                out_index = int(i);
                temp.outfile = temp.args[i + 1];
                break;
            }

        }

        //Consume redirection tokens;
        if (out_index > -1) temp.args.erase(temp.args.begin() + out_index, temp.args.begin() + out_index + 2);

        //Push task into task list:
        task_list.push_back(temp);
    }

}

void task_manager::run_input(){
    auto ptr = this->task_list[0].getArgs();
    pid_t pid = fork();
    if(pid == 0) {

    	execvp(ptr[0], ptr);
    	
    }
    int status;
    waitpid(-1,&status, 0);
}



