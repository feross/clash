#include "job-parser.h"
#include "job.h"

#include <cstring>
#include <cctype>

// bool JobParser::IsCompleteJob(string& job_str, Environment& env) {
//     Parse(job_str, env, false);
//     //throws if incomplete, because we need to determine which character caused
//     return true;
// }

bool JobParser::IsPartialJob(string& job_str, Environment& env) {
    try {
        Parse(job_str, env, false);
    } catch (IncompleteParseException& ipe) {
        //incomplete job given, need more lines
        return true;
    } //intentionally allow fatal errors through
    return false;
}

ParsedJob JobParser::Parse(string& job_str, Environment& env) {
    return Parse(job_str, env, true);
}

//overview: will parse individual pipelines... and keep continuing until whole
//string is consumed (throwing if last pipeline is incomplete)

ParsedJob JobParser::Parse(string& job_str, Environment& env, bool should_execute) {
    string job_str_copy(job_str);
    ParsedJob job;
    while(!job_str_copy.empty()) {
        ParsedPipeline pipeline = ParsePipeline(job_str_copy, env, should_execute);
        pipeline.remaining_job_str = string(job_str_copy);
        if (pipeline.commands.size() > 0) job.pipelines.push_back(pipeline);
    }
    return job;
}


ParsedPipeline JobParser::ParsePipeline(string& job_str_copy, Environment& env, bool should_execute) {

    ParsedCommand command;
    ParsedPipeline pipeline;

    string partial_word = string();
    bool next_word_redirects_out = false;
    bool next_word_redirects_in = false;
    bool quote_word = false;
    bool glob_current_word = false;

    while(true) {
        int match_index = strcspn(job_str_copy.c_str(), " \t\n;|<>~$'`\"\\*?[&");
        if (match_index != 0) partial_word.append(job_str_copy.substr(0,match_index));

        //word breaking
        if (match_index == job_str_copy.size() ||
          string("\t\n ;|<>").find(job_str_copy[match_index]) != string::npos) {
            if (partial_word.size() > 0 || quote_word) { //word exists

                vector<string> words_to_add;
                if (glob_current_word) {
                    glob_current_word = false;
                    words_to_add = FileUtil::GetGlobMatches(partial_word);
                    //always at least one word
                } else words_to_add.push_back(partial_word);
                if (next_word_redirects_in) {
                    command.input_file = words_to_add[0];
                    next_word_redirects_in = false;
                } else if (next_word_redirects_out) {
                    command.output_file = words_to_add[0];
                    next_word_redirects_out = false;
                } else command.words.push_back(words_to_add[0]);

                for (int i = 1; i < words_to_add.size(); i++) {
                    command.words.push_back(words_to_add[i]);
                }
                partial_word = string();
                quote_word = false;
            }
        }

        //command breaking
        if (match_index == job_str_copy.size()) {
            if (command.words.empty() && !pipeline.commands.empty()) {
                throw IncompleteParseException("Incomplete job given, no command break", '|');
            }
            if (!command.words.empty()) pipeline.commands.push_back(command);
        // technically code duplication, but the implementation that avoids
        // this is significantly less clear.
        } else if (string(";|").find(job_str_copy[match_index]) != string::npos) {
            if (command.words.empty()) {
                throw SyntaxErrorParseException(job_str_copy[match_index]);
            }
            pipeline.commands.push_back(command);
            command.clear();
        }

        //redirected words
        if (next_word_redirects_in || next_word_redirects_out) {
            if (match_index == job_str_copy.size()) {
                throw SyntaxErrorParseException("syntax error near unexpected newline");
            } else if (string("\n;|<>").find(job_str_copy[match_index]) != string::npos) {
                if (next_word_redirects_in) {
                    throw SyntaxErrorParseException("no file given for input redirection");
                } else if (next_word_redirects_out) {
                    throw SyntaxErrorParseException("no file given for output redirection");
                }
                throw SyntaxErrorParseException(job_str_copy[match_index]);
            }
        }

        //fully consumed string
        if (match_index == job_str_copy.size()) {
            job_str_copy = string(); //technically job_str_copy.substr(match_index);
            break;
        }
        char matched = job_str_copy[match_index];
        job_str_copy = job_str_copy.substr(match_index + 1);
        if (matched == ';') {
            break; //because can't break within switch
        }
        switch(matched) {
            case '&':
                if (partial_word.empty() && pipeline.commands.size() > 0) {
                    pipeline.commands.back().redirect_stderr = true;
                } else {
                    partial_word.append(1, matched);
                }
                continue;
            case '*':
            case '?':
            case '[': {
                glob_current_word = true;
                partial_word.append(1, matched);
                continue;
            }
            case '\t':
            case ' ':
            case '\n':
            case '|': {
                debug("whitespace, prev_word:%s", partial_word.c_str());
                continue;
            }
            case '<': {
                next_word_redirects_in = true;
                continue;
            }
            case '>': {
                next_word_redirects_out = true;
                continue;
            }
            case '~': {
                if (partial_word.empty() && !quote_word) {
                  partial_word.append(ParseTilde(job_str_copy, env));
                } else {
                  partial_word.append("~");
                }
                continue;
            }
            case '\"': {
                partial_word.append(ParseDoubleQuote(job_str_copy, env, should_execute));
                quote_word = true;
                continue;
            }
            case '\'': {
                partial_word.append(ParseSingleQuote(job_str_copy));
                quote_word = true;
                continue;
            }
            case '`': {
                partial_word.append(ParseBacktick(job_str_copy, env, should_execute));
                continue;
            }
            case '\\': {
                partial_word.append(ParseBackslash(job_str_copy));
                continue;
            }
            case '$': {
                //to word break, places variable to parse back on job_str_copy
                string nonparse_output = ParseVariable(job_str_copy, env);
                if (nonparse_output == string("ambiguous if redirect")) {
                    if (next_word_redirects_in) {
                        throw FatalParseException("Ambiguous input redirection");
                    } else if (next_word_redirects_out) {
                        throw FatalParseException("Ambiguous output redirection");
                    }
                    nonparse_output = string();
                }
                partial_word.append(nonparse_output);
                continue;
            }
            default : {
                throw IncompleteParseException("Matched Unknown character", '?');
            }
        }
    }
    return pipeline;
}


string JobParser::ParseDoubleQuote(string& job_str_copy, Environment& env,
        bool should_execute) {
    string quoted = string();
    int match_index;
    while((match_index = strcspn(job_str_copy.c_str(), "\"`$\\")) != job_str_copy.size()) {
        char matched = job_str_copy[match_index];
        quoted.append(job_str_copy.substr(0,match_index));
        job_str_copy = job_str_copy.substr(match_index + 1);
        switch(matched) {
            case '\"': {
                return quoted;
            }
            case '`': {
                quoted.append(ParseBacktick(job_str_copy, env, should_execute));
                continue;
            }
            case '\\': {
                quoted.append(ParseBackslash(job_str_copy, '\"'));
                continue;
            }
            case '$': {
                string nonparsed_output = ParseVariable(job_str_copy, env);
                if (nonparsed_output != string("ambiguous if redirect")) {
                    quoted.append(nonparsed_output);
                }
                continue;
            }
        }
    }
    // unmatched "
    throw IncompleteParseException("Incomplete job given, no valid closing quote (\")", '\"');
}

string JobParser::ParseSingleQuote(string& job_str_copy) {
    int match_index = strcspn(job_str_copy.c_str(), "\'");
    string quoted = job_str_copy.substr(0,match_index);
    if (match_index != job_str_copy.size()) {
        job_str_copy = job_str_copy.substr(match_index + 1);
        return quoted;
    }
    // unmatched '
    throw IncompleteParseException("Incomplete job given, no valid closing quote (')", '\'');
}

string JobParser::ParseBackslash(string& job_str_copy, char mode) {
    string quoted = job_str_copy.substr(0,1);
    if (mode == ' ') {
        job_str_copy = job_str_copy.substr(1);
        if (quoted != "\n") return quoted;
        else {
            if (job_str_copy.empty()) {
                throw IncompleteParseException("Incomplete job given, no valid closing (\\)", '\\');
            } else {
                return string();
            }
        }
    }
    //"$", "`" (backquote), double-quote, backslash, or newline;
    if (mode == '\"') {
        string valid_matches("$`\"\\\n");
        if (valid_matches.find(quoted) == string::npos) {
            string unmodified("\\");
            return unmodified;
        }
        job_str_copy = job_str_copy.substr(1);
        return quoted;
    }
    //only backtick (inside, will parse as new job)
    if (mode == '`') {
        string valid_matches("`");
        if (valid_matches.find(quoted) == string::npos) {
            string unmodified("\\");
            return unmodified;
        }
        job_str_copy = job_str_copy.substr(1);
        return quoted;
    }
    throw IncompleteParseException("Unknown backslash mode", '\\');
}

string JobParser::ParseVariable(string& job_str_copy, Environment& env) {
    char first_var_char = job_str_copy[0];
    string variable_name;
    if (string("*?#").find(first_var_char) != string::npos ||
        isdigit(first_var_char)) {
        variable_name = string(1, first_var_char);
        job_str_copy = job_str_copy.substr(1);
    } else if (first_var_char =='{') {
        int match_index = job_str_copy.find_first_of("}");
        if (match_index == string::npos) {
            throw IncompleteParseException("Incomplete job given, no valid closing (})", '}');
        } 
        variable_name = job_str_copy.substr(1,match_index-1); //skip first {
        job_str_copy = job_str_copy.substr(match_index + 1);
    } else if (isalpha(first_var_char)) {
        int match_index = job_str_copy.find_first_not_of(
          "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
        variable_name = job_str_copy.substr(0,match_index);
        job_str_copy = job_str_copy.substr(match_index);
    } else {
        string unmodified("$");
        return unmodified;
    }
    string var_value = env.get_variable(variable_name);
    job_str_copy = var_value + job_str_copy;

    /**
     * Bash for some reason is not OK with redirect to a variable with a space,
     * even though it's supposed to parse into words... and it's fine with you
     * giving the same thing as multiple words.
     * Better handling option:
     * return word containing full variable, and parse in the returning context
     * for spaces only (would be relatively easy, even just split - TODO)
     */
    if (var_value.find(" \t\n") != string::npos) {
        return string("ambiguous if redirect");
    }
    return string();
}


string JobParser::ParseBacktick(string& job_str_copy, Environment& env,
        bool should_execute) {
    string quoted = string();
    int match_index;
    while((match_index = strcspn(job_str_copy.c_str(), "`\\")) != job_str_copy.size()) {
        char matched = job_str_copy[match_index];
        debug("strcspn loc str:%s, char:%c", job_str_copy.c_str() + match_index, matched);
        quoted.append(job_str_copy.substr(0,match_index));
        job_str_copy = job_str_copy.substr(match_index + 1);
        if (matched == '`') {
            if (should_execute) {
                string command_output_str;
                try {
                    vector<int> fds = FileUtil::CreatePipe();
                    int read = fds[0];
                    int write = fds[1];
                    ParsedJob parsed_job = Parse(quoted, env);
                    Job(parsed_job, env).RunAndWait(STDIN_FILENO, write);
                    FileUtil::CloseDescriptor(write);
                    command_output_str = FileUtil::ReadFileDescriptor(read);
                    FileUtil::CloseDescriptor(read);
                    //bash special cases this (compare bash printf v.s. echo
                    //in command subs - should be different, isn't)
                    int end_pos = command_output_str.size() - 1;
                    if (command_output_str[end_pos] == '\n') {
                        command_output_str = command_output_str.substr(0, end_pos);
                    }
                } catch (IncompleteParseException& ipe) {
                    string subcommand_err_message("command substitution: ");
                    subcommand_err_message.append(ipe.what());
                    throw FatalParseException(subcommand_err_message);
                } catch (FatalParseException& fpe) {
                    string subcommand_err_message("command substitution: ");
                    subcommand_err_message.append(fpe.what());
                    throw FatalParseException(subcommand_err_message);
                }
                job_str_copy = command_output_str + job_str_copy;
                debug("new string:%s\n", job_str_copy.c_str());
            }
            return string();
        } else {
            quoted.append(ParseBackslash(job_str_copy, '`'));
        }
    }
    throw IncompleteParseException("Incomplete job given, no valid closing backtick (`)", '`');
}

string JobParser::ParseTilde(string& job_str_copy, Environment& env) {
    int match_index = job_str_copy.find_first_of("/\t\n ;|<>");
    string matched_str = job_str_copy.substr(0,match_index);
    if (matched_str.size() == 0) {
        //just expand tilde w/o username. As per spec, this is default var
        return env.get_variable("HOME");
    }
    string home_dir = ProcUtil::GetUserHomeDirectory(matched_str);
    if (home_dir == "") {
        //no user found, just use literal tilde & consume no input
        return string("~");
    }
    job_str_copy = job_str_copy.substr(match_index);
    return home_dir;
}
