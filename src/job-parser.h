#pragma once

#include "string-util.h"

using namespace std;

struct ParsedCommand {
    vector<string> words;
    string inputFile;
    string outputFile;
};

struct ParsedPipeline {
    vector<ParsedCommand> commands;
};

struct ParsedJob {
    vector<ParsedPipeline> pipelines;
};

class JobParser {
    public:
        static ParsedJob Parse(string& job_str);
    private:
};
