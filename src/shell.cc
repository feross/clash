#include "shell.h"

Shell::Shell(int argc, char* argv[]) {
    env.set_variable("0", argv[0]);
    env.set_variable("?", "0");

    int command_flag_index = -1;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--command") == 0) {
            command_flag_index = i;
            break;
        }
    }

    int vars_start = command_flag_index + 2;
    int total_vars = argc - vars_start - 1;
    if (total_vars < 0) {
        total_vars = 0;
    }
    env.set_variable("#", to_string(total_vars));

    string all_args;
    for (int i = 0; i < argc - vars_start; i++) {
        string argument = string(argv[vars_start + i]);
        env.set_variable(to_string(i), argument);
        if (i != 1) all_args.append(" ");
        if (i != 0) all_args.append(argument);
    }
    env.set_variable("*", all_args);
}

bool Shell::ParseString(string& job_str) {
    try {
        if (job_parser.IsPartialJob(job_str, env)) {
            // Return failure if this An incomplete job is considered a failure, since the job
            return false; // treat partial jobs as failure
        } else {
            ParsedJob parsed_job = job_parser.Parse(job_str, env);
            jobs.push_back(Job(parsed_job, env));
        }
    } catch (exception& e) {
        printf("-clash: %s\n", e.what());
        return false; //did parse, into nothing (was invalid)
    }
    return true; //completely & validly parsed
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

bool Shell::RunJobsAndWait() {
    for (Job& job : jobs) {
        try {
            debug("%s", job.ToString().c_str());
            job.RunAndWait();
        } catch (exception& err) {
            printf("-clash: %s\n", err.what());
            jobs.clear();
            return false;
        }
    }
    jobs.clear();
    return true;
}

int Shell::StartRepl() {
    bool isTTY = isatty(STDIN_FILENO);
    debug("isTTY: %d", isTTY);
    string remaining_job_str;
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

        try {
            if (job_parser.IsPartialJob(remaining_job_str, env)) {
                // Incomplete job, so get more input from user
                continue;
            } else {
                ParsedJob parsed_job = job_parser.Parse(remaining_job_str, env);
                jobs.push_back(Job(parsed_job, env));
            }
        } catch (exception& e) {
            // Parsed the complete job, but it was invalid
            printf("-clash: %s\n", e.what());
        }

        RunJobsAndWait();
        remaining_job_str = string();
    }

    if (!remaining_job_str.empty()) {
        printf("-clash: syntax error: unexpected end of file\n");
        return 2;
    }

    return stoi(env.get_variable("?"));
}
