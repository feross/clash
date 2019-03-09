#pragma once

#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "file-util.h"
#include "log.h"

using namespace std;

/**
 * Permission flags to enable user+group+global read, and user write. In other
 * words, these are the flags that would get set with "chmod 644".
 */
static const mode_t kDefaultPermissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

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
