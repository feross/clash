#include "job.h"

Job::Job(string& job_str, Environment& env) :
    original_job_str(job_str), env(env) {
    
    ParsedJob parsed_job = job_parser.Parse(job_str, env);

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

void Job::RunAndWait() {
    for (Pipeline& pipeline : pipelines) {
        pipeline.RunAndWait();
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
