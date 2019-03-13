#include "job.h"

Job::Job(ParsedJob& parsed_job, Environment& env) : env(env) {

    for (ParsedPipeline& parsed_pipeline : parsed_job.pipelines) {
        vector<Command> commands;

        for (ParsedCommand& parsed_command : parsed_pipeline.commands) {
            Command command(parsed_command, env);
            commands.push_back(command);
        }

        Pipeline pipeline(commands);
        pipelines.push_back(pipeline);
    }
}

void Job::RunAndWait(int job_source, int job_sink) {
    for (Pipeline& pipeline : pipelines) {
        pipeline.RunAndWait(job_source, job_sink);
    }
}

string Job::ToString() {
    string result = "Job:";
    for (Pipeline& pipeline : pipelines) {
        result += "\n  " + pipeline.ToString();
    }
    return result;
}

// void Job::Parse(string& job_str) {

//     //TODO: Merge this into the constructor... AND MOVE THE JOB_PARSER HERE
//     // -> The division between the job parser & job doesn't really make sense
//     // and feels very pass-through in general...

//     ParsedJob parsed_job = job_parser.Parse(job_str);

//     for (ParsedPipeline& parsed_pipeline : parsed_job.pipelines) {
//         vector<Command> commands;

//         for (ParsedCommand& parsed_command : parsed_pipeline.commands) {
//             Command command(parsed_command, env);
//             commands.push_back(command);
//         }

//         Pipeline pipeline(commands);
//         pipelines.push_back(pipeline);
//     }
// }
