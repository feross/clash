/**
 * This class exposes a friendly interface for parsing a command line
 * argument string (i.e. char* argv[]) into a more useful structure. The
 * user must specify expected argument names, argument descriptions, and
 * expected argument types (bool, int, or string).
 *
 * Example:
 *
 *      int main(int argc, char* argv[]) {
 *          Arguments args("Hello World - A hello world CLI program");
 *          args.RegisterBool("help", "Print help message");
 *          try {
 *              args.Parse(argc, argv);
 *          } catch (exception& err) {
 *              printf("Error: %s\n", err.what());
 *              return 1;
 *          }
 *          if (args.GetBool("help")) {
 *              printf("%s\n", args.GetHelpText().c_str());
 *              return 0;
 *          }
 *      }
 */

#pragma once

#include <algorithm>
#include <exception>
#include <map>
#include <string>
#include <vector>

#include "string-util.h"

using namespace std;

class Arguments {
    public:
        /**
         * Construct a command line argument parser.
         *
         * @param intro Help text to describe the purpose of the program
         */
        Arguments(string intro) : intro(intro) {};

        /**
         * Register a named boolean command line argument with the given name
         * and description. Defaults to false.
         *
         * Boolean argument flags appear alone with no following value
         * (e.g. "--mybool").
         *
         * @param name        The full name (e.g. --mybool) of the argument
         * @param description Friendly description of the argument's purpose
         */
        void RegisterBool(const string name, const string description);

        /**
         * Register a named integer command line argument with the given name
         * and description. Dsefaults to -1.
         *
         * Integer argument flags must be followed immediately by a space and
         * number (e.g. "--myint 42").
         *
         * @param name        The full name (e.g. --myint) of the argument
         * @param description Friendly description of the argument's purpose
         */
        void RegisterInt(const string name, const string description);

        /**
         * Register a named string command line argument with the given name
         * and description. Defaults to "".
         *
         * String arguments must be followed immediately by a space and a string
         * (e.g. "--mystring hello").
         *
         * @param name        The full name (e.g. --mystring) of the argument
         * @param description Friendly description of the argument's purpose
         */
        void RegisterString(const string name, const string description);

        /**
         * Register a one character command alias with the given name. For
         * example, the command alias "-h" could be registered as a shortcut for
         * the full command name "--help".
         *
         * @param alias The one-character alias (e.g. 'h')
         * @param name The full command name that the alias maps to (e.g "help")
         */
        void RegisterAlias(const char alias, const string name);

        /**
         * Parse the user-provided command line argument string, usually
         * obtained directly from the arguments to main().
         *
         * May throw an exception if the command line argument string is
         * malformed, or missing a required value (e.g. for a named string or
         * integer argument which requires a value).
         *
         * @throw ArgumentsException
         *
         * @param argc Number of command line arguments
         * @param argv Array of command line argument strings
         */
        void Parse(int argc, char* argv[]);

        /**
         * Return the value of the boolean argument with the given name.
         *
         * @throw ArgumentsException
         *
         * @param  argument name
         * @return argument value
         */
        bool GetBool(const string name);

        /**
         * Return the value of the integer argument with the given name.
         *
         * @throw ArgumentsException
         *
         * @param  argument name
         * @return argument value
         */
        int GetInt(const string name);

        /**
         * Return the value of the string argument with the given name.
         *
         * @throw ArgumentsException
         *
         * @param  argument name
         * @return argument value
         */
        const string& GetString(const string name);

        /**
         * Return a vector of the unnamed "extra" arguments included in the
         * command line argument string.
         *
         * Unnamed arguments are useful for command line programs which accept
         * an unbounded number of command line arguments, often as the last
         * arguments to a program.
         *
         * Unnamed arguments lack the two dash prefix (--) which distinguishes
         * named arguments (e.g. --mybool) and are not associated with named
         * arguments as their value (e.g. in "--myint 42", the "42" is not an
         * unnamed argument).
         *
         * Example of 3 unnamed arguments:
         *
         *   "./program --mybool --myint 42 unnamed1 unnamed2 unnamed3"
         *
         * @return vector of unnamed argument strings
         */
        const vector<string>& GetUnnamed();

        /**
         * Returns the program's help text, including the "intro" string
         * specified in the constructor, as well as a generated list of
         * argument names, descriptions, and types.
         *
         * @return string of command line help text
         */
        string GetHelpText();

    private:
        /**
         * Argument descriptions. Maps argument names to string descriptions
         * that describe the purpose of the arguments.
         */
        map<string, string> descriptions;

        /**
         * Argument value map. Maps arguments of each type (bool, int, string)
         * to their actual values after the argument information (i.e. argc and
         * argv) has been parsed.
         */
        map<string, bool> bool_args;
        map<string, int> int_args;
        map<string, string> string_args;

        /**
         * Maps from command alias to full command name and vide versa. Used to
         * translate a command alias like e.g. "-h" to a full command name like
         * e.g. "--help" and vice versa.
         */
        map<string, string> alias_to_name;
        map<string, string> name_to_alias;

        /**
         * Vector of the unnamed "extra" arguments included in the command line
         * argument string.
         */
        vector<string> unnamed_args;

        /**
         * Text to describe the purpose of the program. Used to generate the
         * program's help text.
         */
        string intro;
};

class ArgumentsException : public exception {
    public:
        ArgumentsException(const string& message): message(message) {}
        ArgumentsException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
};
