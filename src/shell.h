#pragma once

#include <fstream>
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <string>

#include "job.h"
#include "job-parser.h"

using namespace std;

class ShellException : public exception {
    public:
        ShellException(const string& message): message(message) {}
        ShellException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
};

class Shell {
    public:
        Shell(Environment& env);
        bool ParseStringIntoJob(string& job_str);
        bool ParseString(string& job_str);

        int RunJobsAndWait();

        bool ParseFile(const string& file_path);
        int StartRepl();
    private:
        Environment& env;
        JobParser job_parser;
        vector<Job> jobs_to_run;
};

