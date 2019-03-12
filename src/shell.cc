#include "shell.h"

bool Shell::ParseStringIntoJobs(string& job_str) {
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
    // string remaining_job_str(job_str);
    // while (remaining_job_str.size() > 0) {
    //     if (jobs_to_run.empty() || jobs_to_run.back().complete) { //short circ
    //         jobs_to_run.push_back(Job());
    //     }
    //     Job& last_job = jobs_to_run.back();
    //     remaining_job_str = last_job.IngestLine(remaining_job_str);
    // }
    // if (jobs_to_run.empty() || jobs_to_run.back().complete) {
    //     return true; //completed, ready to run
    // }
    // return false;
    string remaining_job_str(job_str);
    //TODO: I think b/c we only parse completed commands, there can only be one job
    //UNLESS someone uses this parse mutliple times without running the parsed jobs
    try {
        // printf("char end:%c:htis", job_str[job_str.size() -1]);
        jobs_to_run.push_back(Job(job_str, env));
    } catch (IncompleteParseException& ipe) {
        //incomplete job given, need more lines
        // printf("\nFAILED PARSING\n");
        return false;
    } catch (FatalParseException& fpe) {
        printf("Error: %s\n", fpe.what());
        job_str = string();
        return true;
    }
    return true; //completely parsed
}

// void Shell::ParseStringIntoJobs(const char * job_str) {
//     const string job_string = job_str;
//     ParseStringIntoJobs(job_string);
// }

void Shell::RunJobsAndWait() {
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

    ParseStringIntoJobs(job_str);
}

void Shell::StartRepl() {
    string remaining_job_str = string();
    while (true) {
        string prompt("% ");
        if (remaining_job_str.length() > 0) prompt = string("> ");
        char* line = readline(prompt.c_str());
        if (line == NULL) {
            break;
        }
        remaining_job_str.append(line);
        remaining_job_str.append("\n");
        if (ParseStringIntoJobs(remaining_job_str)) {
            RunJobsAndWait();
            remaining_job_str = string();
        }
        free(line);
    }
}
