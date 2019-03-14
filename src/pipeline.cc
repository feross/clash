#include "pipeline.h"

void Pipeline::RunAndWait(int pipeline_source, int pipeline_sink) {
    if (commands.size() == 0) {
        return;
    }

    if (commands.size() == 1) {
        commands[0].Run(pipeline_source, pipeline_sink);
        commands[0].Wait();
        return;
    }

    vector<int> fds = FileUtil::CreatePipe();
    int command_sink = fds[1];
    int command_source = fds[0];

    commands[0].Run(pipeline_source, command_sink);
    FileUtil::CloseDescriptor(command_sink);

    for (size_t i = 1; i < commands.size() - 1; i++) {
        command_source = fds[0];
        fds = FileUtil::CreatePipe();
        command_sink = fds[1];

        commands[i].Run(command_source, command_sink);

        FileUtil::CloseDescriptor(command_source);
        FileUtil::CloseDescriptor(command_sink);
    }

    command_source = fds[0];
    commands[commands.size() - 1].Run(command_source, pipeline_sink);
    FileUtil::CloseDescriptor(command_source);

    for (Command& command : commands) {
        command.Wait();
    }
}

string Pipeline::ToString() {
    string result = "Pipeline:";
    for (Command& command : commands) {
        result += "\n    " + command.ToString();
    }
    return result;
}
