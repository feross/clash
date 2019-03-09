#pragma once

#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "file-util.h"
#include "log.h"

using namespace std;

class Command {
    public:
        Command(vector<string> words, string inputFile, string outputFile) :
            inputFile(inputFile), outputFile(outputFile), words(words), pid(0) {}
        void Run(int source, int sink);
        void Wait();
        string ToString();
        string inputFile;
        string outputFile;
    private:
        vector<string> words;
        pid_t pid;
};
