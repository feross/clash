#pragma once

#include "string-util.h"
#include "environment.h"

using namespace std;


// struct JobStringsToInject {
//     string raw_job_str;
//     int pos_word;
//     bool quote_word;
// };

//TODO: decide if this is best
// really, this is where the subjob lives - anywhere else would
// be "living" on a higher level than actually is true internally
// struct InjectedWord {
//     string word;
//     vector<struct JobStringsToInject> subjobs;
//     void clear() {
//         subjobs.clear();
//     }
// };

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

struct ParsedPipeline {
    vector<ParsedCommand> commands;
    string remaining_job_str; //these must be reparsed after running, huge number
    // of tests rely on this TODO don't love, but might require to match behavior
    void clear() {
        commands.clear();
        remaining_job_str.clear();
    };
};

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
        IncompleteParseException(const string& message): message(message) {}
        IncompleteParseException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
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

// class SyntaxErrorParseException: public FatalParseException {
//  public:
//   SyntaxErrorParseException() throw() {}
//   SyntaxErrorParseException(const std::string& message) throw() : HTTPProxyException(message) {}
// };

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
    public:
        static bool IsPartialJob(string& job_str, Environment& env);
        // Can only parse complete commands
        // behavior not defined if parse incompelte commands
        // (will likely just throw, and maybe run subcommands)
        //COMMENT: if called, will attempt to parse - including any command
        // substitutions that might be necessary to do that parsing.  This
        // may have side effects, so you should call IsPartialJob first to
        // avoid executing command substitutions within partial commands
        static ParsedJob Parse(string& job_str, Environment& env);
    private:
        static ParsedJob Parse(string& job_str, Environment& env,
            bool should_execute);
        static ParsedPipeline ParsePipeline(string& job_str_copy,
            Environment& env, bool should_execute);
        static string SwitchParsingTarget(char matched, string& loc, Environment& env);
        static string ParseDoubleQuote(string& job_str_copy, Environment& env,
            bool should_execute);
        static string ParseSingleQuote(string& job_str_copy);
        static string ParseBackslash(string& job_str_copy, char mode = ' ');
        static string ParseVariable(string& job_str_copy, Environment& env);
        static string ParseBacktick(string& job_str_copy, Environment& env,
            bool should_execute);
        static string ParseTilde(string& job_str_copy, Environment& env);

};
