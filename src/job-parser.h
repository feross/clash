#pragma once

#include "string-util.h"
#include "environment.h"
#include "proc-util.h"

using namespace std;


/**
 * Set of structs containing the information necessary to spawn a job
 */

/**
 * Parsed Command describes where a command, and whether it shoudl redirect
 * input or output, potentially including stderr
 */
struct ParsedCommand {
    ParsedCommand() : redirect_stderr(false) {}
    vector<string> words;
    string input_file;
    string output_file;
    bool redirect_stderr;
    void clear() {
        words.clear();
        input_file.clear();
        output_file.clear();
    };
};

/**
 * Parsed Pipeline describes where a Pipeline, and any remaining
 * job string that will need to be reparsed after we run this pipeline
 * (e.g. variables that may now be set)
 */
struct ParsedPipeline {
    vector<ParsedCommand> commands;
    string remaining_job_str; //after we run, any later pipelines must be reparsed
    void clear() {
        commands.clear();
        remaining_job_str.clear();
    };
};

/**
 * Parsed Job describes a job, whether it is completely cnstructed, and
 * contains a useful utility for printing the content of a job.
 */
struct ParsedJob {
    vector<ParsedPipeline> pipelines;
    bool complete = true; //TODO: for cases where we have multiple lines, e.g. \\n, unmatched quotes, etc
    void clear() {
      pipelines.clear();
      complete = true; //TODO: false in future
    };
    void print() {
      debug("%s", "job:");
      for (ParsedPipeline& pipeline : pipelines) {
              debug("%s", "  pipeline:");
          for (ParsedCommand& command : pipeline.commands) {
              debug("%s", "    command:");
              debug("      input_file:%s", command.input_file.c_str());
              debug("      output_file:%s", command.output_file.c_str());
              for (string& command_str : command.words) {
                  debug("      word: %s", command_str.c_str());
              }
          }
      }
    }
};

class IncompleteParseException : public exception {
    public:
        IncompleteParseException() {}
        IncompleteParseException(const string& message, char ch): message(message),
            problem_char(ch) {}
        IncompleteParseException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
        char unmatched_char() const noexcept { return problem_char; }        
    private:
        string message;
        char problem_char;
};

class FatalParseException : public exception {
    public:
        FatalParseException() {}
        FatalParseException(const string& message): message(message) {}
        FatalParseException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
};

class SyntaxErrorParseException : public FatalParseException {
    public:
        SyntaxErrorParseException(const string& message): message(message) {}
        SyntaxErrorParseException(const char* message): message(message) {}
        SyntaxErrorParseException(const char ch) {
            message = ("syntax error near unexpected token \'" +
                string(1, ch) + "\'").c_str();
        }
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
};

class JobParser {
    /**
     * Static parsing class, primarily using the call stack to keep track of
     * any matching requirements for characters.  Due to this design, we
     * throw on parsing errors, and generally require a valid command for
     * successful parsing (we throw helpful exceptions to describe why parsing
     * failed - including exceptions to specify paritally-complete jobs v.s.
     * invalid jobs)
     */
    public:
        /**
         * Attempts to determine whether a job_str fully describes a job
         * or whether more input is needed before a complete job
         * can be constructed.
         * 
         * @param job_str - string describing job, to be parsed
         * @param env - the environment which has variables we may substitute
         *
         * @return bool - whether this is a partial job
         * throws - FatalParseException if this job is syntactically invalid 
         */
        static bool IsPartialJob(string& job_str, Environment& env);

        /**
         * Parse method will completely parse a command, including any command
         * substitutions that must be run to determine the command.
         * Should use IsPartialJob first to determine if this job is valid,
         * to avoid running command substitutions.
         *
         * @param job_str - string describing job, to be parsed
         * @param env - the environment which has variables we may substitute
         *
         * @return ParsedJob - the fully parsed job content
         * throws - FatalParseException if this job is syntactically invalid 
         * throws - IncompletePraseException if this job incomplete
         */
        static ParsedJob Parse(string& job_str, Environment& env);

    private:
        /**
         * Internal parse method that takes a boolean determining whether to 
         * execute command substitutions or not.
         *
         * @param job_str - string describing job, to be parsed
         * @param env - the environment which has variables we may substitute
         * @return ParsedJob - the fully parsed job content
         *
         * throws - FatalParseException if this job is syntactically invalid 
         * throws - IncompletePraseException if this job incomplete
         */
        static ParsedJob Parse(string& job_str, Environment& env,
            bool should_execute);

        /**
         * Parses an individual pipeline from command.  Was originally intended
         * to allow parsing of individual commands to be run before parsing
         * subsequent commands (since this may affect variable substitutions), 
         * but experimentation with bash suggests that parsing is approximately
         * checked up-front (except subcommands).
         */
        static ParsedPipeline ParsePipeline(string& job_str_copy,
            Environment& env, bool should_execute);
        /**
         * Will attempt to consume input until a valid closing quote is found.
         * This includes performing any interal substitutions necessary, though
         * will only attempt command substitutions if we know we are planning
         * on running the full job.
         *
         * @param job_str_copy - a reference to a copy of the string we are
         *      parsing over.  Consumes job_string
         * @param env - the environment which has variables we may substitute
         * @return - string content inside the quotes
         */
        static string ParseDoubleQuote(string& job_str_copy, Environment& env,
            bool should_execute);
        /**
         * Will attempt to consume input until a valid closing backtick is found
         * If should_execute is true, we will actually run the subcommand (which
         * is part of parsing - it's generating the input to be parsed).  The
         * output is pushed back on the front of the string we are parsing, to
         * be "reparsed from scratch" including things that may change the
         * behavior & interpretation of characters outside the backticks
         * 
         * @param job_str_copy - a reference to a copy of the string we are
         *      parsing over.  Consumes job_string
         * @param env - the environment which has variables we may substitute
         * @return - Returns an empty string, since all output must be reparsed
         * from scratch.
         */
        static string ParseBacktick(string& job_str_copy, Environment& env,
            bool should_execute);
        static string ParseSingleQuote(string& job_str_copy);
        static string ParseBackslash(string& job_str_copy, char mode = ' ');
        static string ParseVariable(string& job_str_copy, Environment& env);
        static string ParseTilde(string& job_str_copy, Environment& env);

};
