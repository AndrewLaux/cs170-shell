#include "task_manager.h"


//*************************************************************************
// -- Function: Default constructor --
// Creates a singleton task manager. Initializes correct PATH location for
// executables.
task_manager::task_manager() {
    cmd_list = std::vector<std::string>();
    ready_list = std::vector<task>();
}

void task_manager::update_command(char *const cmd){

    //Check that input was not just a newline:
    if(strncmp(cmd, "\n", 512) == 0) throw std::runtime_error("Bad input\n");

    //TODO: Check that taskmanager is ready for a new command input.
    ready_list = std::vector<task>();

    //Take new cmd line and parse into whitespace seperated commands:
    cmd_list = std::vector<std::string>();
    std::string cmd_line = std::string(cmd);
    std::stringstream strm(cmd_line);
    std::string token;
    while (strm >> token) {
        this->cmd_list.push_back(token);
    }

    //Check for background flag:
    bool background = false;
    if (cmd_list.back() == "&"){
        background = true;
        cmd_list.pop_back();
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

        //Set background flag if found:
        if(background == true) temp.background = true;

        //Push task into task list:
        ready_list.push_back(temp);
    }

}

//*************************************************************************
// -- Function: run_input --
// Executes input using seperate processes for each command block. Capable 
// of piping output and input between child processes. In the event of an 
// error should return to main program loop and allow for addition commands.
void task_manager::run_input(){
    

    int status;
    int i = 0;
    pid_t pid;
    bool do_not_wait = ready_list[0].background;

    //Special instrucitons for cd
    if(ready_list.size() == 1) {
        if(ready_list[0].executable == "cd"){
            auto ptr = this->ready_list[0].getArgs();
            status = chdir(ptr[1]);
            if(status < 0) {
                perror("ERROR");
                exit(EXIT_FAILURE);
            }
            return;
        }
        
    }

    //Create pipes for each set of command blocks in ready_list.
    int numPipes = ready_list.size() - 1;
    int pipefds[2*numPipes];
    for(i = 0; i < (numPipes); i++){
        if(pipe(pipefds + i*2) < 0) {
            perror("ERROR");
            exit(EXIT_FAILURE);
        }
    }

    //Create child processes for each command in ready_list:
    int j = 0;
    for (int index = 0; index < ready_list.size(); index++) {

        //Create child:
        pid = fork();

        //In child:
        if(pid == 0) {

            //Use dup2 to reference fd from stdout, if not last block:
            if(index < ready_list.size() - 1){
                if(dup2(pipefds[j + 1], 1) < 0){
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                }
            }

            //Use dup2 to reference fd from stdin if not first block:
            if(j != 0 ){
                if(dup2(pipefds[j-2], 0) < 0){
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                }
            }


            //Use dup2 to reference infile in first block if specified:
            if(index == 0) {
                if(ready_list[index].infile != "none") {
                    close(STDIN_FILENO);
                    int file = open(ready_list[index].infile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    if(dup(file) < 0){
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                    }
                    close(file);
                }
            }

            //Use dup2 tp ref outfile in last block if specified:
            if(index == ready_list.size() - 1) {
                if(ready_list[index].infile != "none") {
                    close(STDOUT_FILENO);
                    int file = open(ready_list[index].infile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    if(dup(file) < 0){
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                    }
                    close(file);
                }
            }

            //Close all fds in child:
            for(i = 0; i < 2*numPipes; i++){
                    close(pipefds[i]);
            }

            //Generate char* arg lists for execution:
            auto ptr = this->ready_list[index].getArgs();
            status = execvp(ptr[0], ptr);      
            if( status < 0 ){
                    perror("ERROR");
                    exit(EXIT_FAILURE);
            }

        } else if(pid < 0){
            perror("error");
            exit(EXIT_FAILURE);
        }

        j+=2;
    }
    
    //Close all pipes in parent process:
    for(i = 0; i < 2 * numPipes; i++){
        close(pipefds[i]);
    }

    //If not background, wait for children:
    if(!do_not_wait){
        for (i = 0; i < numPipes + 1; i++)
            wait(&status);
    }
    
}



