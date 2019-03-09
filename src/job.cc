#include "job.h"

Job::Job(string job_str) : job_str(job_str) {}

void Job::RunAndWait() {
    Parse();
    for (Pipeline& pipeline : pipelines) {
        pipeline.Run();
        pipeline.Wait();
    }
}

string Job::ToString() {
    Parse();
    string result = "Job:";
    for (Pipeline& pipeline : pipelines) {
        result += "\n  " + pipeline.ToString();
    }
    return result;
}

void Job::Parse() {
    ParsedJob parsed_job = job_parser.Parse(job_str);

    for (ParsedPipeline& parsed_pipeline : parsed_job.pipelines) {
        vector<Command> commands;

        for (ParsedCommand& parsed_command : parsed_pipeline.commands) {
            Command command(parsed_command.words, parsed_command.inputFile,
                parsed_command.outputFile);
            commands.push_back(command);
        }

        Pipeline pipeline(commands);
        pipelines.push_back(pipeline);
    }
}
