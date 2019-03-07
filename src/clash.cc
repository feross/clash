#include "clash.h"

#include "arguments.h"
#include "util.h"

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

    warn("Hello world! %d", 123);

    return EXIT_SUCCESS;
}