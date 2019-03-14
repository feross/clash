/**
 * A set of one or more "commands", where the input from one command flows into
 * the next command.
 */

#pragma once

#include <string>
#include <vector>

#include "command.h"
#include "file-util.h"
#include "log.h"

using namespace std;

class Pipeline {
    public:
        /**
         * Create a new pipeline of commands. Input from one command flows into
         * the next command.
         */
        Pipeline(vector<Command> commands) : commands(commands) {}

        /**
         * Run the pipeline and wait for all the commands contained within to
         * finish running. This runs all the commands that make up the pipeline
         * simultaneously and blocks until all of them finish running.
         *
         * The given arguments, pipeline_source and pipeline_sink, are used to
         * configure the stdin of the first process and the stdout of the last
         * process, respectively.
         *
         * @param pipeline_source The file descriptor to use as stdin to the first
         *                        command in the pipeline
         * @param pipeline_sink   The file descriptor to use as the stdout of the
         *                        final command in the pipeline
         */
        void RunAndWait(int pipeline_source = STDIN_FILENO, int pipeline_sink = STDOUT_FILENO);

        /**
         * Returns a readable string representation of the pipeline, including
         * the contained commands.
         */
        string ToString();
    private:
        /**
         * List of all the commands that make up this pipeline.
         */
        vector<Command> commands;
};
