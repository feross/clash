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
        return EXIT_FAILURE;
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

    vector<string> unnamed_args = args.get_unnamed();
    // Shell script mode
    if (unnamed_args.size() > 0) {
        string script_name = unnamed_args[0];

        string line;
        ifstream script_file(script_name);
        if (!script_file.is_open()) {
            printf("-clash: error while opening file");
        }
        while (getline(script_file, line)) {
            Job job(line);
            try {
                job.RunAndWait();
                debug("%s", job.ToString().c_str());
            } catch (exception& err) {
                printf("-clash: %s\n", err.what());
            }
        }
        if (script_file.bad()) {
            printf("-clash: error while reading file");
        }

        return EXIT_SUCCESS;
    }

    // Interactive mode
    char * line;
    while (true) {
        line = readline("% ");
        if (line == NULL) {
            break;
        }

        Job job(line);
        try {
            job.RunAndWait();
            debug("%s", job.ToString().c_str());
        } catch (exception& err) {
            printf("-clash: %s\n", err.what());
        }

        free(line);
    }

    return EXIT_SUCCESS;
}