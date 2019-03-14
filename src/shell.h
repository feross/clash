/**
 * Shell is the top-level class that represents an instance of the Clash
 * command line program. It's primary purpose is to process user input and run
 * it as a series of jobs.

 * In our system, we use the following terminology:
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

#include <fstream>
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <string>

#include "environment.h"
#include "job.h"
#include "job-parser.h"

using namespace std;

class Shell {
    public:
        Shell(int argc, char* argv[]);

        /**
         * Parse a job string, which is a set of one or more "pipelines" to be
         * run sequentially, one after the other, separated by semicolons or
         * newlines. The parsed job is added to the list of jobs to be executed
         * when RunJobsAndWait() is called.
         *
         * Returns a boolean indicating whether the given job string
         * constitutes a valid and complete job. Jobs which end without closting
         * all quotes and properly finishing all sequences of special commands
         * like output file redirections are considered invalid.
         *
         * @param  job_str The string that defines the sequence of commands to run
         * @return         True if the job was valid and parsed completely
         *                 False otherwise.
         */
        bool ParseString(string& job_str);

        /**
         * Read and parse a shell script file, which is a file with a newline-
         * separated list of jobs to execute. The parsed job is added to the
         * list of jobs to be executed when RunJobsAndWait() is called.
         *
         * @param  file_path Filesystem path to the shell script to run
         * @return           True if the job was valid and parsed completely
         *                   False otherwise.
         */
        bool ParseFile(const string& file_path);

        /**
         * Run all the jobs which have been previously parsed, sequentially, one
         * after the other. Blocks until all the jobs are finished running.
         * After the jobs are finished running, the list of pending jobs is
         * cleared.
         *
         * @return True if the jobs in the pending job queue were all executed
         *         without errors.
         */
        bool RunJobsAndWait();

        /**
         * Start a Read-Eval-Print loop which continually prompts the user for
         * jobs from stdin. If stdin is a TTY, then a helpful prompt is printed
         * a incomplete job strings are detected and the user is helpfully
         * prompted to complete their original command on a new line.
         *
         * If stdin is not a TTY, then no prompt is printed and lines are
         * continuously read from stdin until EOF is reached. If there is in
         * an incomplete job when EOF is reached, then an error is printed.
         *
         * @return the exit code of last command int he
         */
        int StartRepl();
    private:
        /**
         * Object that represents the local shell environment that all jobs will
         * run within. Stores environment variables and the PATH cache.
         */
        Environment env;

        /**
         * Job string parser. Aids us in the complex task of parsing the user's
         * job strings into a useful structured form.
         */
        JobParser job_parser;

        /**
         * List of pending jobs to run when RunJobsAndWait() is called.
         */
        vector<Job> jobs;
};

class ShellException : public exception {
    public:
        ShellException(const string& message): message(message) {}
        ShellException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
};
