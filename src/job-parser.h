#pragma once

#include "string-util.h"

using namespace std;

struct ParsedCommand {
    vector<string> words;
    string input_file;
    string output_file;
};

struct ParsedPipeline {
    vector<ParsedCommand> commands;
};

struct ParsedJob {
    vector<ParsedPipeline> pipelines;
};

class ParseException : public exception {
    public:
        ParseException(const string& message): message(message) {}
        ParseException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
};

class JobParser {
    public:
        static ParsedJob Parse(string& job_str);
    private:
};
