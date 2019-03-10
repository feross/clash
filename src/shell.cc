#include "shell.h"

void Shell::RunJobAndWait(const string& job_str) {
    // TODO: splitting on newlines should be handled by the parser
    vector<string> lines = StringUtil::Split(job_str, "\n");
    for (string& line : lines) {
        Job job(line, env);
        try {
            debug("%s", job.ToString().c_str());
            job.RunAndWait();
        } catch (exception& err) {
            printf("-clash: %s\n", err.what());
        }
    }
}

void Shell::RunJobAndWait(const char * job_str) {
    const string job_string = job_str;
    RunJobAndWait(job_string);
}

void Shell::RunFileAndWait(const string& file_path) {
    string job_str;
    ifstream file(file_path);
    if (!file.is_open()) {
        printf("-clash: error while opening file %s", file_path.c_str());
        return;
    }

    string line;
    while (getline(file, line)) {
        job_str += line + "\n";
    }

    if (file.bad()) {
        printf("-clash: error while reading file %s", file_path.c_str());
        return;
    }

    RunJobAndWait(job_str);
}

void Shell::StartRepl() {
    char * line;
    while (true) {
        line = readline("% ");
        if (line == NULL) {
            break;
        }
        RunJobAndWait(line);
        free(line);
    }
}
