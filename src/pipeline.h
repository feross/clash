#pragma once

#include <string>
#include <vector>

#include "command.h"
#include "file-util.h"
#include "log.h"

using namespace std;

class Pipeline {
    public:
        Pipeline(vector<Command> commands) : commands(commands) {}
        void RunAndWait(int pipeline_source = STDIN_FILENO, int pipeline_sink = STDOUT_FILENO);
        string ToString();
    private:
        vector<Command> commands;
};
