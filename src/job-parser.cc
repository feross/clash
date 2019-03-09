#include "job-parser.h"

ParsedJob JobParser::Parse(string& job_str) {
    ParsedJob job;

    vector<string> pipeline_strs = StringUtil::Split(job_str, ";"); //TODO: fix, maybe escaped ('\")
    for (string& pipeline_str : pipeline_strs) {
        ParsedPipeline pipeline;

        vector<string> command_strs = StringUtil::Split(pipeline_str, "|"); //TODO: fix, maybe escaped ('\")
        for (string& command_str : command_strs) {
            ParsedCommand command;
            // printf("%s\n", command_str.c_str());

            vector<string> split_command = StringUtil::Split(command_str, ">"); //TODO: fix, maybe escaped ('\")
            command_str = split_command[0];
            if (split_command.size() >= 2) {
                for (int i = 1; i < split_command.size(); i++) {
                    int start_pos = split_command[i].find_first_not_of(' ');
                    int end_pos = split_command[i].find_first_of(' ', start_pos);
                    command.outputFile = split_command[i].substr(start_pos, end_pos);
                    if (end_pos < split_command[i].size()) {
                        command_str.push_back(' ');
                        command_str.append(split_command[i].substr(end_pos));
                    }
                }
            }

            split_command = StringUtil::Split(command_str, "<"); //TODO: fix, maybe escaped ('\")
            command_str = split_command[0];
            if (split_command.size() >= 2) {
                for (int i = 1; i < split_command.size(); i++) {
                    int start_pos = split_command[i].find_first_not_of(' ');
                    int end_pos = split_command[i].find_first_of(' ', start_pos);
                    command.inputFile = split_command[i].substr(start_pos, end_pos);
                    if (end_pos < split_command[i].size()) {
                        command_str.push_back(' ');
                        command_str.append(split_command[i].substr(end_pos));
                    }
                }
            }

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