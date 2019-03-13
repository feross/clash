#include "clash.h"

LogType LOG_LEVEL = INFO;

int main(int argc, char* argv[]) {
    Arguments args(INTRO_TEXT);
    args.RegisterBool("help", "Print help message");
    args.RegisterAlias("h", "help");

    args.RegisterBool("verbose", "Show debug logs");
    args.RegisterAlias("v", "verbose");

    args.RegisterBool("quiet", "Hide all logs except errors");
    args.RegisterAlias("q", "quiet");

    args.RegisterString("command", "Run command");
    args.RegisterAlias("c", "command");

    try {
        args.Parse(argc, argv);
    } catch (ArgumentsException& err) {
        error("%s", err.what());
        return 2;
    }

    if (args.get_bool("quiet")) {
        LOG_LEVEL = ERROR;
    } else if (args.get_bool("verbose")) {
        LOG_LEVEL = DEBUG;
    }

    if (args.get_bool("help")) {
        printf("%s\n", args.get_help_text().c_str());
        return EXIT_SUCCESS;
    }

    Shell shell;

    string command = args.get_string("command");
    if (!command.empty()) {
        try {
            // TODO: This doesn't actually throw right now, I think. Make it
            // do so since we want to handle printing the error message and
            // setting the status code here
            shell.ParseStringIntoJob(command);
            shell.RunJobsAndWait();
        } catch (exception& err) {
            printf("-clash: %s\n", err.what());
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    vector<string> unnamed_args = args.get_unnamed();
    if (unnamed_args.size() > 0) {
        const string& file_path = unnamed_args[0];
        try {
            shell.ParseFile(file_path);
            shell.RunJobsAndWait();
        } catch (exception& err) {
            printf("-clash: %s\n", err.what());
            return 127;
        }
        return EXIT_SUCCESS;
    }

    shell.StartRepl();
    return EXIT_SUCCESS;
}
