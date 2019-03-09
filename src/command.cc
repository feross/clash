#include "command.h"

void Command::Run(int source, int sink) {
    pid = FileUtil::CreateProcess();
    if (pid == 0) {
        if (!inputFile.empty()) {
            if (source != 0) {
                FileUtil::CloseDescriptor(source);
            }
            source = FileUtil::OpenFile(inputFile, O_RDONLY);
        }

        if (source != 0) {
            FileUtil::DuplicateDescriptor(source, STDIN_FILENO);
        }

        if (!outputFile.empty()) {
            if (sink != 0) {
                FileUtil::CloseDescriptor(sink);
            }
            sink = FileUtil::OpenFile(outputFile, O_WRONLY | O_CREAT | O_TRUNC,
                kDefaultPermissions);
        }

        if (sink != 0) {
            FileUtil::DuplicateDescriptor(sink, STDOUT_FILENO);
        }

        char * argv[words.size() + 1];
        for (size_t i = 0; i < words.size(); i++) {
            argv[i] = const_cast<char *>(words[i].c_str());
        }
        argv[words.size()] = NULL;

        execvp(argv[0], argv);
        fprintf(stderr, "-clash: %s: command not found\n", argv[0]);
        exit(0);
        return;
    }
}

void Command::Wait() {
    // TODO: check for error
    waitpid(pid, NULL, 0);
}

string Command::ToString() {
    string result = "Command:";
    for (string& word : words) {
        result += " " + word;
    }
    return result;
}