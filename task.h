#ifndef task_h
#define task_h



#include <string>
#include <cstring>
#include <vector>


//*************************************************************************
// -- Class: task --
// A task is a program name combined with a list of its argunments and in
// or out redirection. It containes relavant information relating to the 
// program's process ID and whether or not it should run in the background.
class task {

public: //Publicly accessible data: ****************************************

    std::string executable;
    std::string infile;
    std::string outfile;
    std::vector<std::string> args;
    bool background;
    char** arg_ptr;

public: //Public functions: ***********************************************

    task() : infile("none"), outfile("none") { arg_ptr = NULL; }
   
    char** getArgs() {
        arg_ptr = new char*[args.size() + 2];
	arg_ptr[0] = new char[32];
	strncpy(arg_ptr[0], executable.c_str(), 32);
        for (size_t i = 0; i < args.size(); i++) {
            arg_ptr[i + 1] = new char[32];
            strncpy(arg_ptr[i + 1], args[i].c_str(), 32);
        }
        arg_ptr[args.size() + 1] = NULL;

        return arg_ptr;
    }

    const char* getExecutable() {
	    return executable.c_str();
    }
    task(const task& ref) {
        this->executable = ref.executable;
        this->infile = ref.infile;
        this->outfile = ref.outfile;
        this->args = ref.args;
        this->background = ref.background;
        this->arg_ptr = NULL;
    }

    ~task() {
        if (arg_ptr != NULL) {
            for (size_t i = 0; i < args.size() + 1; i++) {
                delete[] arg_ptr[i];
            }
            delete[] arg_ptr;

        }
    }

};


#endif
