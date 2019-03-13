#include "shell.h"

// TODO: this should throw an exception that is handled by clash.cc and the printing
// should happen there
bool Shell::ParseStringIntoJob(string& job_str) {
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
    //TODO: I think b/c we only parse completed commands, there can only be one job
    //UNLESS someone uses this parse mutliple times without running the parsed jobs
    try {
        if (job_parser.IsPartialJob(job_str, env)) {
            return false; // incomplete job
        } else {
            ParsedJob parsed_job = job_parser.Parse(job_str, env);
            jobs_to_run.push_back(Job(parsed_job, env));
        }
    } catch (exception& e) {
        printf("-clash: %s\n", e.what());
        return true; // did parse, into nothing (was invalid)
    }
    return true; //completely & validly parsed
}

bool Shell::ParseString(string& job_str) {
    try {
        if (job_parser.IsPartialJob(job_str, env)) {
            return false; // treat partial jobs as failure
        } else {
            ParsedJob parsed_job = job_parser.Parse(job_str, env);
            jobs_to_run.push_back(Job(parsed_job, env));
        }
    } catch (exception& e) {
        printf("-clash: %s\n", e.what());
        return false; //did parse, into nothing (was invalid)
    }
    return true; //completely & validly parsed
}

// void Shell::ParseStringIntoJobs(const char * job_str) {
//     const string job_string = job_str;
//     ParseStringIntoJobs(job_string);
// }

int Shell::RunJobsAndWait() {
    for (Job& job : jobs_to_run) {
        try {
            debug("%s", job.ToString().c_str());
            job.RunAndWait();
        } catch (exception& err) {
            printf("-clash: %s\n", err.what());
            jobs_to_run.clear();
            return -1;
        }
    }
    jobs_to_run.clear();

    return 0;
}

bool Shell::ParseFile(const string& file_path) {
    string job_str;
    ifstream file(file_path);
    if (!file.is_open()) {
        printf("-clash: %s: No such file or directory\n", file_path.c_str());
        return false;
    }

    string line;
    while (getline(file, line)) {
        job_str += line + "\n";
    }

    if (file.bad()) {
        printf("-clash: %s: Error reading file\n", file_path.c_str());
        return false;
    }

    return ParseString(job_str);
}

int Shell::StartRepl() {
    // for (string& match : FileUtil::GetGlobMatches("src/[a-c]lash.cc")) {
    //     debug("match: %s", match.c_str());
    // }
    bool isTTY = isatty(STDIN_FILENO);
    debug("isTTY: %d", isTTY);
    string remaining_job_str = string();
    while (true) {
        char * prompt = NULL;
        if (isTTY) {
            prompt = (char *) (remaining_job_str.length() == 0 ? "% " : "> ");
        }

        char* line = readline(prompt);
        if (line == NULL) {
            break;
        }

        remaining_job_str.append(line);
        remaining_job_str.append("\n");
        free(line);

        // printf("str:%s\n", remaining_job_str.c_str());
        if (ParseStringIntoJob(remaining_job_str)) {
            RunJobsAndWait();
            // TODO: make this string cleared in a consistent place
            remaining_job_str = string();
        }
    }

    if (!remaining_job_str.empty()) {
        printf("-clash: syntax error: unexpected end of file\n");
        return 2;
    }
    return atoi(env.get_variable("?").c_str());
}

