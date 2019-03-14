#include "job-parser.h"
#include "job.h"

#include <cstring>
#include <cctype>

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

//so we have a "parse pipeline" function, v.s. a "consume string" function.
//consume string will output

ParsedJob JobParser::Parse(string& job_str, Environment& env, bool should_execute) {
    string job_str_copy(job_str);
    ParsedJob job;
    while(!job_str_copy.empty()) {
        ParsedPipeline pipeline = ParsePipeline(job_str_copy, env, should_execute);
        pipeline.remaining_job_str = string(job_str_copy);
        if (pipeline.commands.size() > 0) job.pipelines.push_back(pipeline);
    }
    // for (ParsedPipeline& pipeline : job.pipelines) {
    //     printf("remaining job_str:%s:\n", pipeline.remaining_job_str.c_str());
    // }
    // printf("remaining job_str:%s:\n", job_str_copy.c_str());
    // job.print();
    return job;
}


ParsedPipeline JobParser::ParsePipeline(string& job_str_copy, Environment& env, bool should_execute) {
    // string& job_str_copy = job_str;
    // const char* job_str_start = job_str_copy.c_str();
    // string& job_str_moved = job_str_copy;
    // job_str_copy = job_str_copy.substr(3);
    // printf("moved:%s:\n", job_str_moved.c_str());
    // printf("copy :%s:\n", job_str_copy.c_str());
    // exit(0);
    ParsedCommand command;
    ParsedPipeline pipeline;
    // ParsedJob job;

    //TODO: could just rebuild command out here, i.e. every time we're about to
    //discard something, or returned a word, build string from that.

    // InjectedWord iword;

    string partial_word = string();
    bool next_word_redirects_out = false;
    bool next_word_redirects_in = false;
    bool quote_word = false;
    bool glob_current_word = false;
    //TODO: probably switch to "find_first_of" and "find_first_not_of" which do same thing I think
    while(true) {
        int match_index = strcspn(job_str_copy.c_str(), " \t\n;|<>~$'`\"\\*?[&");
        if (match_index != 0) partial_word.append(job_str_copy.substr(0,match_index));

        // if (match_index != job_str_copy.size() &&
        //     string("*?[").find(job_str_copy[match_index]) != string::npos) {
        //     glob_current_word = true;
        //     partial_word.append(1, job_str_copy[match_index]);
        // }
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
                throw IncompleteParseException("Incomplete job given, no command break");
            }
            if (!command.words.empty()) pipeline.commands.push_back(command);
        // technically code duplication, but the implementation that avoids
        // this is significantly less clear.  Thoughts?
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
            job_str_copy = string(); //technically job_str_copy.substr(match_index); //MODJOBSTR
            break;
        }
        char matched = job_str_copy[match_index];
        job_str_copy = job_str_copy.substr(match_index + 1); //MODJOBSTR
        if (matched == ';') {
            break;
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
            case ';': { //TODO: remove if breaking above (i.e. turning this into
                // a pipeline-parsing function to be called repeatedly)
                //TODO: doesn't work at end of line
                //TODO: need to introduce a variable
                // that tracks whether the last command ended with |
                // otherwise can't distinguish echo good;
                // "echo good;"" vs. "echo bad |"
                // the latter should be fine, but must ask
                // for more characters
                // job.pipelines.push_back(pipeline);
                // pipeline.clear();
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
                  //TODO: tries substitution on ``, probably shouldn't
                  // but slightly ambigious re: "reparse from scratch"
                  //easily fixed with storing "prev_match" but feels hacky
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
            //TODO: append to list of "commands to run"
            //In this case, DO parse output
            case '`': {
                // struct JobStringsToInject inner_job;
                // inner_job.raw_job_str = ParseBacktick(job_str_copy, env);
                // inner_job.pos_word = partial_word.size();
                // inner_job.pos_command = command.words.size();
                // inner_job.pos_pipeline = pipeline.commands.size();
                // inner_job.pos_job = job.pipelines.size();
                // inner_job.quote_word = false;
                // backtick_jobs.push_back(inner_job);
                partial_word.append(ParseBacktick(job_str_copy, env, should_execute));
                continue;
            }
            case '\\': {
                partial_word.append(ParseBackslash(job_str_copy));
                continue;
            }
            case '$': {
                //TODO: In this case DO parse output
                string nonparse_output = ParseVariable(job_str_copy, env);
                if (nonparse_output == string("ambigious if redirect")) {
                    if (next_word_redirects_in) {
                        throw FatalParseException("Ambiguous input redirection");
                    } else if (next_word_redirects_out) {
                        throw FatalParseException("Ambiguous output redirection");
                    }
                    nonparse_output = string();
                }
                partial_word.append(nonparse_output);
                // partial_word.append(ParseVariable(job_str_copy, env));
                continue;
            }
            default : {
                throw IncompleteParseException("Matched Unknown character");
            }
        }
    }
    //command breaking already handled, but must add pipeline to jobs still
    // if (pipeline.commands.size() > 0) job.pipelines.push_back(pipeline);
    // job.print();
    // printf("starting_job_now:%s:\n", job_str_start);
    // printf("remaining pipeline str:%s:\n", job_str_copy.c_str());
    return pipeline;
}


// string JobParser::SwitchParsingTarget(char matched, string& job_str_copy, Environment& env) {
//     switch(matched) {
//         //TODO: Somehwat disgusting idea that avoids addint state and should still
//         //maintain correctness : have a sentinel that is INVALID to parse
//         //be returned from "quoted" backticks, such that we just
//         //look up - in order - these sentinels which CANNOT initially appear
//         //through any other means, and run the saved commands in that order
//     }
// }

string JobParser::ParseDoubleQuote(string& job_str_copy, Environment& env,
        bool should_execute) {
    string quoted = string();
    int match_index;
    while((match_index = strcspn(job_str_copy.c_str(), "\"`$\\")) != job_str_copy.size()) {
        char matched = job_str_copy[match_index];
        quoted.append(job_str_copy.substr(0,match_index));
        job_str_copy = job_str_copy.substr(match_index + 1); //MODJOBSTR
        switch(matched) {
            case '\"': {
                return quoted;
            }
            case '`': {
                //TODO: append to list of "commands to run"
                //In this case, DON'T parse output
                // struct JobStringsToInject inner_job;
                // inner_job.raw_job_str = ParseBacktick(job_str_copy, env);
                quoted.append(ParseBacktick(job_str_copy, env, should_execute));
                continue;
            }
            case '\\': {
                quoted.append(ParseBackslash(job_str_copy, '\"'));
                continue;
            }
            case '$': {
                //TODO: In this case DON'T parse output
                quoted.append(ParseVariable(job_str_copy, env));
                continue;
            }
        }
    }
    // unmatched "
    throw IncompleteParseException("Incomplete job given, no valid closing quote (\")");
}

string JobParser::ParseSingleQuote(string& job_str_copy) {
    int match_index = strcspn(job_str_copy.c_str(), "\'");
    string quoted = job_str_copy.substr(0,match_index);
    if (match_index != job_str_copy.size()) {
        job_str_copy = job_str_copy.substr(match_index + 1); //MODJOBSTR
        return quoted;
    }
    // unmatched '
    throw IncompleteParseException("Incomplete job given, no valid closing quote (')");
}

string JobParser::ParseBackslash(string& job_str_copy, char mode) {
    string quoted = job_str_copy.substr(0,1);
    if (mode == ' ') {
        job_str_copy = job_str_copy.substr(1); //MODJOBSTR
        if (quoted != "\n") return quoted;
        else {
            if (job_str_copy.empty()) {
                throw IncompleteParseException("Incomplete job given, no valid closing (/)");
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
            // unmodified.append(quoted);
            return unmodified;
        }
        job_str_copy = job_str_copy.substr(1); //MODJOBSTR
        return quoted;
    }
    //only backtick (inside, will parse as new job)
    if (mode == '`') {
        string valid_matches("`");
        if (valid_matches.find(quoted) == string::npos) {
            string unmodified("\\");
            //newer TODO: bash actually seems to do the old behavior - e.g. echo `\\` is a complete command according to bash
            // -> so should actually allow escaping of \ by \.
            // unmodified.append(quoted); //TODO: WRONG to leave in, I think - e.g. double backslash, shouldn't consume second backslack
            //TODO: confirm this was right to comment out (i.e. instead output single
            // backslash & leave next char inside the job_str_copy in case it's special
            // job_str_copy = job_str_copy.substr(1); //tested - bash does this, not what the spec says
            return unmodified;
        }
        //TODO: bash actually behaves incorrectly/differently than ousterhout's code
        //namely, bash will consider something like echo `echo \\` a complete command, even
        //though according to spec that first backslashs should be ignored while scanning for end
        //and the second one should cause the last ` not to match
        job_str_copy = job_str_copy.substr(1); //MODJOBSTR
        return quoted;
    }
    throw IncompleteParseException("Unknown backslash mode");
}

//TODO: parse whitespace on return IF AND ONLY IF this is in main unquoted lines
//    ---> ideally, in main thread, but then have a temp-state thing I don't love
string JobParser::ParseVariable(string& job_str_copy, Environment& env) { //TODO: push at front & reparse (may introduce words)
    char first_var_char = job_str_copy[0];
    string variable_name;
    if (string("*?#").find(first_var_char) != string::npos ||
        isdigit(first_var_char)) {
        variable_name = string(1, first_var_char);
        job_str_copy = job_str_copy.substr(1); //MODJOBSTR
    } else if (first_var_char =='{') {
        int match_index = job_str_copy.find_first_of("}");
        if (match_index == string::npos) {

            throw IncompleteParseException("Incomplete job given, no valid closing (})");
        }
        variable_name = job_str_copy.substr(1,match_index-1); //skip first
        job_str_copy = job_str_copy.substr(match_index + 1); //MODJOBSTR
    } else if (isalpha(first_var_char)) {
        int match_index = job_str_copy.find_first_not_of(
          "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
        variable_name = job_str_copy.substr(0,match_index);
        job_str_copy = job_str_copy.substr(match_index); //MODJOBSTR
    } else {
        string unmodified("$");
        return unmodified;
    }
    string var_value = env.get_variable(variable_name);
    job_str_copy = var_value + job_str_copy;
    //Bash for some reason is not OK with redirect to a variable with a space,
    //even though it's supposed to parse into words... and it's fine with you
    //giving the same thing as multiple words
    if (var_value.find(" \t\n") != string::npos) {
        return string("ambigious if redirect");
    }
    return string();
}

//TODO: parse whitespace on return IF AND ONLY IF this is in main unquoted lines
//    ---> ideally, in main thread, but then have a temp-state thing I don't love
string JobParser::ParseBacktick(string& job_str_copy, Environment& env,
        bool should_execute) { //TODO: push at front & reparse (may introduce words)
    string quoted = string();
    int match_index;
    while((match_index = strcspn(job_str_copy.c_str(), "`\\")) != job_str_copy.size()) {
        char matched = job_str_copy[match_index];
        debug("strcspn loc str:%s, char:%c", job_str_copy.c_str() + match_index, matched);
        quoted.append(job_str_copy.substr(0,match_index));
        job_str_copy = job_str_copy.substr(match_index + 1); //MODJOBSTR
        if (matched == '`') {
            if (should_execute) {
                string command_output_str;
                try {
                    vector<int> fds = FileUtil::CreatePipe();
                    int read = fds[0];
                    int write = fds[1];
                    ParsedJob parsed_job = Parse(quoted, env);
                    Job(parsed_job, env).RunAndWait(STDIN_FILENO, write);  //TODO redirect this to put inside our string
                    // Job(quoted, env).RunAndWait(STDIN_FILENO, STDOUT_FILENO);  //TODO redirect this to put inside our string
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
                    //TODO: probably should probably define more different errors for each
                    string subcommand_err_message("command substitution: ");
                    subcommand_err_message.append(ipe.what());
                    throw FatalParseException(subcommand_err_message);
                } catch (FatalParseException& fpe) {
                    //TODO: maybe just allow this to propogate?  Idk feels like I should
                    //say that it's within a subcommand
                    string subcommand_err_message("command substitution: ");
                    subcommand_err_message.append(fpe.what());
                    throw FatalParseException(subcommand_err_message);
                }
                job_str_copy = command_output_str + job_str_copy; //MODJOBSTR
                debug("new string:%s\n", job_str_copy.c_str());
            }
            // TODO: we MUST inject here if we want to reparse from scratch
            // otherwise can't know if we've completed a command
            // (e.g. echo "`echo \"` isn't complete unless we run `echo \"`)
            return string(); //TODO: we can actually just return the command
            //to be injected, `` never returns any actual parsed content
        } else {
            quoted.append(ParseBackslash(job_str_copy, '`'));
        }
    }
    throw IncompleteParseException("Incomplete job given, no valid closing backtick (`)");
}

string JobParser::ParseTilde(string& job_str_copy, Environment& env) {
    int match_index = job_str_copy.find_first_of("/\t\n ;|<>");
    string matched_str = job_str_copy.substr(0,match_index);
    if (matched_str.size() == 0) {
        //just expand tilde w/o username. As per spec, this is default var
        return env.get_variable("HOME");
    }
    string home_dir = FileUtil::GetUserHomeDirectory(matched_str);
    if (home_dir == "") {
        //no user found, just use literal tilde & consume no input
        return string("~");
    }
    job_str_copy = job_str_copy.substr(match_index); //MODJOBSTR
    return home_dir;
}
