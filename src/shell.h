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
        void RunJobAndWait(const string& job_string);
        void RunJobAndWait(const char * job_string);

        void RunFileAndWait(const string& file_path);
        void StartRepl();
    private:
};

