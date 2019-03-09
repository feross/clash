#include "command.h"

// typedef function<void()> SetupCallback;

void Command::Run(int source, int sink) {
    debug("running");
    pid = FileUtil::CreateProcess();
    if (pid == 0) {
        // child_setup_callback();

        // redirectInput(p.input);
        if (source != 0) {
            FileUtil::DuplicateDescriptor(source, STDIN_FILENO);
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
        printf("-clash: %s: command not found\n", argv[0]);
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