#include "command.h"

Command::Command(ParsedCommand parsed_command, Environment& env) :
    pid(0), env(env) {

    words = parsed_command.words;
    input_file = parsed_command.input_file;
    output_file = parsed_command.output_file;
}

void Command::Run(int source, int sink) {
    pid = FileUtil::CreateProcess();
    if (pid == 0) {
        if (!input_file.empty()) {
            if (source != 0) {
                FileUtil::CloseDescriptor(source);
            }
            source = FileUtil::OpenFile(input_file);
        }

        if (source != 0) {
            FileUtil::DuplicateDescriptor(source, STDIN_FILENO);
        }

        if (!output_file.empty()) {
            if (sink != 0) {
                FileUtil::CloseDescriptor(sink);
            }
            sink = FileUtil::OpenFile(output_file, O_WRONLY | O_CREAT | O_TRUNC);
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
    if (!input_file.empty()) {
        result += " [input_file: " + input_file + "]";
    }
    if (!output_file.empty()) {
        result += " [output_file: " + output_file + "]";
    }
    return result;
}
