#include "clash.h"

LogType LOG_LEVEL = INFO;

int main(int argc, char* argv[]) {
    Arguments args(INTRO_TEXT);
    args.RegisterBool("help", "Print help message");
    args.RegisterBool("verbose", "Show debug logs");
    args.RegisterBool("quiet", "Hide all logs except errors");

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

    char * line;
    while (true) {
        line = readline("% ");
        if (line == NULL) {
            break;
        }

        Job job(line);
        debug("%s", job.ToString().c_str());

        try {
            job.RunAndWait();
        } catch (exception& err) {
            printf("-clash: %s\n", err.what());
        }


        free(line);
    }

    return EXIT_SUCCESS;
}