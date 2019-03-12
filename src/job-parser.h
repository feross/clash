#pragma once

#include "string-util.h"

using namespace std;

// struct ParsedCommand {
//     vector<string> words;
//     string input_file;
//     string output_file;
//       void clear() {
//         words.clear();
//         input_file.clear();
//         output_file.clear();
//       };
// };

// struct ParsedPipeline {
//     vector<ParsedCommand> commands;
//     void clear() { commands.clear(); };
// };

// struct ParsedJob {
//     vector<ParsedPipeline> pipelines;
//     bool complete = true; //TODO: for cases where we have multiple lines, e.g. \\n, unmatched quotes, etc
//     void clear() {
//       pipelines.clear();
//       complete = true; //TODO: false in future
//     };
//     void print() {
//       debug("%s", "job:");
//       for (ParsedPipeline& pipeline : pipelines) {
//               debug("%s", "  pipeline:");
//           for (ParsedCommand& command : pipeline.commands) {
//               debug("%s", "    command:");
//               debug("      input_file:%s", command.input_file.c_str());
//               debug("      output_file:%s", command.output_file.c_str());
//               for (string& command_str : command.words) {
//                   debug("      word: %s", command_str.c_str());
//               }
//           }
//       }
//     }
// };

class IncompleteParseException : public exception {
    public:
        IncompleteParseException(const string& message): message(message) {}
        IncompleteParseException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
};

// class JobParser {
//     public:
//         static ParsedJob Parse(string& job_str);
//     private:
//         static string SwitchParsingTarget(char matched, string& loc);
//         static string ParseDoubleQuote(string& job_str_copy);
//         static string ParseSingleQuote(string& job_str_copy);
//         static string ParseBackslash(string& job_str_copy, char mode = ' ');
//         static string ParseVariable(string& job_str_copy);
//         static string ParseBacktick(string& job_str_copy);
//         static string ParseTilde(string& job_str_copy);
class FatalParseException : public exception {
    public:
        FatalParseException(const string& message): message(message) {}
        FatalParseException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
};

// };
