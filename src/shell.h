#pragma once

#include <fstream>
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <string>

#include "job.h"

using namespace std;

class Shell {
    public:
        bool ParseStringIntoJobs(string& job_str);
        // void ParseStringIntoJobs(const char * job_str);

        void RunJobsAndWait();

        void ParseFile(const string& file_path);
        void StartRepl();
    private:
        Environment env;
        vector<Job> jobs_to_run;
};

