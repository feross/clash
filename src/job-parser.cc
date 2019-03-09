#include "job-parser.h"

ParsedJob JobParser::Parse(string& job_str) {
    ParsedJob job;

    vector<string> pipeline_strs = StringUtil::Split(job_str, ";");
    for (string& pipeline_str : pipeline_strs) {
        ParsedPipeline pipeline;

        vector<string> command_strs = StringUtil::Split(pipeline_str, "|");
        for (string& command_str : command_strs) {
            ParsedCommand command;

            // TODO: handle multiple whitespaces in a row, different whitespace
            //       types
            // TODO: handle input and output file redirection
            command.words = StringUtil::Split(command_str, " ");

            pipeline.commands.push_back(command);
        }

        job.pipelines.push_back(pipeline);
    }

    return job;
}