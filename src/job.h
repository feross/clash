/**
 * Job class which represents a job to be run by the terminal. Offers
 * functionality for parsing a user-entered string. Handles jobs that are
 * composed of many pipelines and many individual commands. In our system,
 * use the following terminology:
 *
 *   "Command"  - An individual progam to be run, along with its arguments and
 *                optional input and output redirection files.
 *
 *   "Pipeline" - A set of one or more "commands", where the input from one
 *                command flows into the next command.
 *
 *   "Job"      - A set of one or more "pipelines" to be run sequentially, one
 *                after the other.
 */

#pragma once

#include <string>
#include <vector>

#include "command.h"
#include "environment.h"
#include "job-parser.h"
#include "log.h"
#include "pipeline.h"

using namespace std;

class Job {
    public:
        /*
         * Construct a new job based on the text entered on the terminal by the
         * user. This job can be "run", meaning that the pipelines and commands
         * contained within will be executed. The job is parsed and executed
         * in a Bash-like manner, obeying the rules of Bash as much as possible.
         *
         * @param job_str The line (or lines) of text entered on the terminal
         *                by the user.
         */
        Job(ParsedJob& job, Environment& env);
        // Job(char * job_str, Environment& env) :
        //     Job(string(job_str), env) {};

        /**
         * Run the job, including all pipelines and commands contained within.
         * Blocks until the entire job finishes running.
         *
         * May throw an exception if the job string is invalid or if there is
         * an error running the job for any reason (e.g. the program is missing,
         * the system ran out of file descriptors, etc.)
         */
        void RunAndWait(int job_source = STDIN_FILENO, int job_sink = STDOUT_FILENO);

        /**
         * Print a readable string representation of the job, including the
         * contained pipelines and commands.
         */
        string ToString();

    private:
        /**
         * The line (or lines) of text entered on the terminal by the user
         */
        string original_job_str;

        /**
         * Instance of the job parser; used to actually parse the string
         * entered by the user.
         */
        // JobParser job_parser;

        /**
         * A job is composed of one or more pipelines, which are stored here
         * after the job string has been parsed.
         */
        vector<Pipeline> pipelines;

        Environment& env;
};
