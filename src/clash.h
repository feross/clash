/**
 * The entry point into the Clash program.
 */

#pragma once

#include <string>
#include <vector>

#include "arguments.h"
#include "log.h"
#include "shell.h"

using namespace std;

/**
 * Help text for the ./clash command line program.
 */
static const string INTRO_TEXT =
R"(Clash - A Simple Bash-Like Shell

Usage:
    ./clash [options]

If no arguments are present, then an interactive REPL is started. If a single
file name argument is provided, the commands are read from that file. If a "-c"
argument is provided, then commands are read from a string.

Examples:
    Start an interactive REPL.
        ./clash

    Read and execute commands from a file.
        ./clash shell-script.sh

    Read and excute commands from a string.
        ./clash -c "echo hello world"

    Read commands from stdin.
        echo "echo hello from stdin" | ./clash
)";
