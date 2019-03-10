#include "job.h"

Job::Job(string job_str, Environment& env) :
    job_str(job_str), has_parsed(false), env(env) {}

void Job::RunAndWait() {
    Parse();
    for (Pipeline& pipeline : pipelines) {
        pipeline.RunAndWait();
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
    if (has_parsed) {
        return;
    }

    ParsedJob parsed_job = job_parser.Parse(job_str);

    for (ParsedPipeline& parsed_pipeline : parsed_job.pipelines) {
        vector<Command> commands;

        for (ParsedCommand& parsed_command : parsed_pipeline.commands) {
            Command command(parsed_command, env);
            commands.push_back(command);
        }

        Pipeline pipeline(commands);
        pipelines.push_back(pipeline);
    }

    has_parsed = true;
}
