#ifndef task_manager_h
#define task_manager_h

//Includes:
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <exception>
#include <unistd.h>
#include <sys/wait.h>
#include "task.h"

//*************************************************************************
// -- Class: task_manager --
// A persistant singleton object responisble for parsing the input of the 
// user and tracking the executionn of those commands.
class task_manager {

private: //Private data: **************************************************
    std::vector <std::string>cmd_list;
    std::vector <task>task_list;

public: //Public functions: ***********************************************
    task_manager(); 
    void update_command(char *const cmd);
    void run_input();



};

#endif

