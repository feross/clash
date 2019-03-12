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
        void RunAndWait();
        string ToString();

        //TODO: make private w/ methods
        vector<Command> commands;
    private:
};
