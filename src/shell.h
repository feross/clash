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

class Shell {
    public:
        bool ParseStringIntoJob(string& job_str);
        // void ParseStringIntoJobs(const char * job_str);

        void RunJobsAndWait();

        void ParseFile(const string& file_path);
        void StartRepl();
    private:
        Environment env;
        JobParser job_parser;
        vector<Job> jobs_to_run;
};

