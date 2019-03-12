#include "shell.h"

void Shell::ParseJobs(const string& job_str) {
    // TODO: splitting on newlines should be handled by the parser
    // vector<string> lines = StringUtil::Split(job_str, "\n");
    // for (string& line : lines) {
    //     Job job(line, env);
    //     try {
    //         debug("%s", job.ToString().c_str());
    //         job.RunAndWait();
    //     } catch (exception& err) {
    //         printf("-clash: %s\n", err.what());
    //     }
    // }
    string remaining_job_str(job_str);
    while (remaining_job_str.size() > 0) {
        if (jobs_to_run.empty() || jobs_to_run.back().complete) { //short circ
            jobs_to_run.push_back(Job());
        }
        Job& last_job = jobs_to_run.back();
        remaining_job_str = last_job.IngestLine(remaining_job_str);
    }
    if (jobs_to_run.empty() || jobs_to_run.back().complete) {
        return true; //completed, ready to run
    }
    return false;
}

bool Shell::ParseJobs(const char * job_str) {
    const string job_string = job_str;
    ParseJobs(job_string);
}

void Shell::ParseFile(const string& file_path) {
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

    ParseJobs(job_str);
}

void Shell::StartRepl() {
    char * line;
    bool incomplete_job = false;
    while (true) {
        string prompt("% ")
        if (incomplete_job) prompt = string("> ");
        line = readline(prompt);
        if (line == NULL) {
            break;
        }
        incomplete_job = ParseJobs(line);
        free(line);
        if (incomplete_job) continue;
        for (Job& job : jobs_to_run) {
            try {
                debug("%s", job.ToString().c_str());
                job.RunAndWait();
            } catch (exception& err) {
                printf("-clash: %s\n", err.what());
            }
        }
        jobs_to_run.clear();
    }
}
