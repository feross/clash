#pragma once

#include <stdlib.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "environment.h"
#include "job-parser.h"
#include "file-util.h"
#include "log.h"
#include "proc-util.h"

using namespace std;

class Command {
    public:
        Command(ParsedCommand parsed_command, Environment& env);
        void Run(int source, int sink);
        void Wait();
        string ToString();
    private:
        bool RunBuiltin();
        void RunProgram(int source, int sink);

        // vector<struct InjectedWord> injected_words;
        vector<string> words;
        string input_file;
        string output_file;
        bool redirect_stderr;
        pid_t pid;
        Environment& env;
};
