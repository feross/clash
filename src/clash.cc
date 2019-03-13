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

    //defaults case
    Environment env;
    env.set_variable("0", argv[0]);
    env.set_variable("?", "0");

    int c_flag_index = -1;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0
            || strcmp(argv[i], "--command") == 0) {
            c_flag_index = i;
            break;
        }
    }
    // printf("%d\n", argc);
    int vars_start = (c_flag_index + 2);
    int total_vars = argc - vars_start - 1;
    if (total_vars < 0) total_vars = 0;
    string all_together;

    env.set_variable("#", to_string(total_vars));
    //handles both file case (starts @ 1), and -c case
    for (int i = 0; i < argc - vars_start; i++) {
        string argument = string(argv[i + vars_start]);
        env.set_variable(to_string(i), argument);
        if (i != 1) all_together.append(" ");
        if (i != 0) all_together.append(argument);
    }
    env.set_variable("*", all_together);

    Shell shell(env);

    string command = args.get_string("command");
    if (!command.empty()) {
        if (!shell.ParseString(command)) {
            return -1;
        }
        return shell.RunJobsAndWait();
    }

    vector<string> unnamed_args = args.get_unnamed();
    if (unnamed_args.size() > 0) {
        const string& file_path = unnamed_args[0];
        if (!shell.ParseFile(file_path)) {
            return 127;
        }
        return shell.RunJobsAndWait();
    }

    return shell.StartRepl();
}
