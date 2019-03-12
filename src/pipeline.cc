#include "pipeline.h"

void Pipeline::RunAndWait() {
    if (commands.size() == 0) {
        return;
    }

    if (commands.size() == 1) {
        commands[0].Run(STDIN_FILENO, STDOUT_FILENO);
        commands[0].Wait();
        return;
    }

    int fds[2];
    FileUtil::CreatePipe(fds);
    int sink = fds[1];
    int source = fds[0];

    commands[0].Run(STDIN_FILENO, sink);
    FileUtil::CloseDescriptor(sink);

    for (size_t i = 1; i < commands.size() - 1; i++) {
        source = fds[0];
        FileUtil::CreatePipe(fds);
        sink = fds[1];

        commands[i].Run(source, sink);

        FileUtil::CloseDescriptor(source);
        FileUtil::CloseDescriptor(sink);
    }

    source = fds[0];
    commands[commands.size() - 1].Run(source, STDOUT_FILENO);
    FileUtil::CloseDescriptor(source);

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
