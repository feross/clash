#include "command.h"

Command::Command(ParsedCommand parsed_command, Environment& env) :
    pid(0), env(env) {

    words = parsed_command.words;
    input_file = parsed_command.input_file;
    output_file = parsed_command.output_file;
    debug("command words: %d", words.size());
}

void Command::Run(int source, int sink) {
    //TODO: run any injected jobs
    // before doing any of this
    //for (struct InjectedWord& iword : injected_words) {
    //    //RUN & INJECT
    //    //Construct normal "words" using output
    //}
    bool isBuiltin = RunBuiltin();
    if (!isBuiltin) {
        RunProgram(source, sink);
    }
}

void Command::Wait() {
    // TODO: check for error
    waitpid(pid, NULL, 0);
}

string Command::ToString() {
    string result = "Command:";
    for (string word : words) {
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

bool Command::RunBuiltin() {
    string& program = words[0];

    if (program == "cd") {
        if (words.size() == 1) {
            const string& home_directory = env.get_variable("HOME");
            if (!home_directory.empty()) {
                FileUtil::SetCurrentWorkingDirectory(home_directory);
            } else {
                printf("cd: HOME not set");
            }
        } else if (words.size() == 2) {
            FileUtil::SetCurrentWorkingDirectory(words[1]);
        } else {
            printf("cd: Too many arguments\n");
        }
        return true;
    }

    if (program == "pwd") {
        if (words.size() == 1) {
            printf("%s\n", FileUtil::GetCurrentWorkingDirectory().c_str());
        } else {
            printf("pwd: Too many arguments\n");
        }
        return true;
    }

    if (program == "exit") {
        if (words.size() == 1) {
            exit(0);
        } else if (words.size() >= 2) {
            int status;
            try {
                status = stoi(words[1]);
            } catch (const invalid_argument& err) {
                printf("exit: %s: Numeric argument required\n", words[1].c_str());
                status = 2;
            }
            if (words.size() != 2) {
                printf("exit: Too many arguments");
            }
            exit(status);
        } else {
        }
        return true;
    }

    if (program == "printenv") {
        vector<string> variable_strings = env.get_export_variable_strings();
        for (string& variable_string : variable_strings) {
            printf("%s\n", variable_string.c_str());
        }
        return true;
    }

    if (program == "set") {
        if (words.size() == 3) {
            env.set_variable(words[1], words[2]);
        } else if (words.size() < 3) {
            printf("set: Not enough arguments");
        } else {
            printf("set: Too many arguments");
        }
        return true;
    }

    // TODO: hack, remove once parser sends "set" correctly
    if (program.find("=") != string::npos) {
        vector<string> res = StringUtil::Split(program, "=");
        env.set_variable(res[0], res[1]);
        return true;
    }

    if (program == "unset") {
        if (words.size() == 1) {
            printf("unset: Not enough arguments\n");
        } else {
            vector<string> names(words.begin() + 1, words.end());
            for (string& name : names) {
                env.unset_variable(name);
            }
        }
        return true;
    }

    if (program == "export") {
        if (words.size() == 1) {
            printf("export: Not enough arguments\n");
        } else {
            vector<string> names(words.begin() + 1, words.end());
            for (string& name : names) {
                env.export_variable(name);
            }
        }
        return true;
    }

    return false;
}

void Command::RunProgram(int source, int sink) {
    pid = FileUtil::CreateProcess();
    if (pid != 0) {
        return;
    }

    if (!input_file.empty()) {
        source = FileUtil::OpenFile(input_file);
    }

    if (!output_file.empty()) {
        sink = FileUtil::OpenFile(output_file, O_WRONLY | O_CREAT | O_TRUNC);
    }

    FileUtil::DuplicateDescriptor(source, STDIN_FILENO);
    FileUtil::DuplicateDescriptor(sink, STDOUT_FILENO);

    // for (string& match : FileUtil::GetGlobMatches(words[1])) {
    //     debug("match: %s", match.c_str());
    // }

    string program_path = env.FindProgramPath(words[0]);
    if (program_path.empty()) {
        fprintf(stderr, "-clash: %s: command not found\n", words[0].c_str());
        exit(0);
    }
    words[0] = program_path;

    // Build argument array
    char * argv[words.size() + 1];
    for (size_t i = 0; i < words.size(); i++) {
        argv[i] = const_cast<char *>(words[i].c_str());
    }
    argv[words.size()] = NULL;

    // Build environment variable array
    vector<string> variable_strings = env.get_export_variable_strings();
    char * envp[variable_strings.size() + 1];
    for (size_t i = 0; i < variable_strings.size(); i++) {
        envp[i] = const_cast<char *>(variable_strings[i].c_str());
    }
    envp[variable_strings.size()] = NULL;

    execve(argv[0], argv, envp);
    fprintf(stderr, "-clash: %s: permission denied\n", argv[0]);
    exit(0);
}
