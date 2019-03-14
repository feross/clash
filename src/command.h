/**
 * An individual progam to be run, along with its arguments and optional input
 * and output redirection files. Commands can also be shell built-ins like "cd",
 * "set", "unset", "printenv", "export", "exit", "pwd", etc.
 */

#pragma once

#include <stdlib.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "environment.h"
#include "job-parser.h"
#include "file-util.h"
#include "log.h"
#include "proc-util.h"

using namespace std;

class Command {
    public:
        /**
         * Create a new command, which may be a prgoram
         *
         * @param parsed_command Struct which describes all the relevant
         *                       information about the command
         * @param env            The shell environment
         */
        Command(ParsedCommand parsed_command, Environment& env);

        /**
         * Run the given command, which may be a program to run or a shell
         * builtin, using the given file descriptors as a source and sink from
         * which to read and write stdin and stdout/stderr to.
         *
         * This function never blocks or waits for the process to finish running.
         *
         * @param source The file descriptor from which to read stdin
         * @param sink   The file descriptor to write stdout/stderr to
         */
        void Run(int source, int sink);

        /**
         * Block until the command has finished running.
         */
        void Wait();

        /**
         * Returns a readable string representation of the command, including
         * the contained arguments and input and output redirection files
         * (if present).
         */
        string ToString();

    private:
        /**
         * Attempts to run the command as a shell builtin. If the command is
         * recognized as a shell builtin, then the command is run as such. Many
         * shell builtins are supported including "cd", "set", "unset",
         * "printenv", "export", "exit", "pwd", etc.
         *
         * Throws any number of various exception types if the builtin cannot
         * be run because of invalid arguments or system call failure.
         *
         * @return True if the command was recognized and executed as a shell
         *         builtin.
         */
        bool RunBuiltin();

        /**
         * Attempts to run the command as a program, setting up the programs
         * stdin and stdout/stderr to redirect to the given source and sink file
         * descriptors. The program's environment is determined by the state
         * of the passed in Environment instance.
         *
         * Throws any number of various exception types if the program cannot
         * be run because there is no such file, the program is not executable,
         * or system call failure.
         *
         * @param source The source file descriptor, where stdin comes from
         * @param sink   The sink file descrptor, where stdout/stderr goes
         */
        void RunProgram(int source, int sink);

        /**
         * List of all the command arguments.
         */
        vector<string> words;

        /**
         * An optional input file to redirect from.
         */
        string input_file;

        /**
         * An optional output file to redirect from.
         */
        string output_file;

        /**
         * Boolean indicating whether to also redirect stderr to the sink
         * file descriptor.
         */
        bool redirect_stderr;

        /**
         * The process id of the started process.
         */
        pid_t pid;

        /**
         * The shell environment.
         */
        Environment& env;
};
