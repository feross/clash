#include "job-parser.h"
#include "job.h"

#include <cstring>
#include <cctype>

ParsedJob JobParser::Parse(string& job_str, Environment& env) {
    string job_str_copy(job_str);
    ParsedCommand command;
    ParsedPipeline pipeline;
    ParsedJob job;
    // printf("job str:%s", job_str_copy.c_str());
    // int match_index;
    string partial_word = string();
    bool next_word_redirects_out = false;
    bool next_word_redirects_in = false;
    bool quote_word = false;
    int iter = 0;
    //TODO: probably switch to "find_first_of" and "find_first_not_of" which do same thing I think
    while(true) {
        iter++;
        int match_index = strcspn(job_str_copy.c_str(), " \t\n;|<>~$'`\"\\");
        // debug("strcspn loc str:%s, char:%c", job_str_copy.c_str() + match_index, matched);
        // debug("partial_word: %s", partial_word.c_str());
        if (match_index != 0) partial_word.append(job_str_copy.substr(0,match_index));
        // debug("partial_word: %s", partial_word.c_str());
        // debug("new job_str_copy:%s", job_str_copy.c_str());
        // string endword_chars("\t\n ;|<>");

        //word breaking 
        if (match_index == job_str_copy.size() ||
          string("\t\n ;|<>").find(job_str_copy[match_index]) != string::npos) {
            //word break
            if (partial_word.size() > 0 || quote_word) { //TODO: change to var, so works with "" words
                if (next_word_redirects_in) {
                    command.input_file = partial_word;
                    next_word_redirects_in = false;
                } else if (next_word_redirects_out) {
                    command.output_file = partial_word;
                    next_word_redirects_out = false;
                } else command.words.push_back(partial_word);
                partial_word = string();
                quote_word = false;
            }
        }
        //command breaking
        if (match_index == job_str_copy.size()) {
            //relies on short circuit to avoid code duplication
            if (command.words.empty() && !pipeline.commands.empty()) {
                throw IncompleteParseException("Incomplete job given, no command break");
            }
            pipeline.commands.push_back(command);
            // command.clear();
        } else if (string(";|").find(job_str_copy[match_index]) != string::npos) {
            if (command.words.empty()) {
                throw SyntaxErrorParseException(job_str_copy[match_index]);
            }
            // technically code duplication, but the implementation that avoids
            // this is significantly less clear.  Thoughts?
            pipeline.commands.push_back(command);
            command.clear();
        } 
        //redirected words (can't exit before, b/c \n is present)
        if (next_word_redirects_in || next_word_redirects_out) {
            if (match_index == job_str_copy.size() ||
                string("\n;|<>").find(job_str_copy[match_index]) != string::npos) {
                //relies on short circuit (though case where it
                //comes up is impossible) Bad?
                throw SyntaxErrorParseException(job_str_copy[match_index]);
            }
        }
        //fully consumed string
        if (match_index == job_str_copy.size()) {
            debug("\nexitall %d iter: %d\n", match_index, iter);
            break;
        }

        char matched = job_str_copy[match_index];
        job_str_copy = job_str_copy.substr(match_index + 1);

        switch(matched) {
            case '\t':
            case ' ':
            case '\n':
            case '|': {
                debug("whitespace, prev_word:%s", partial_word.c_str());
                continue;
            }
            case ';': { //TODO: doesn't work at end of line
                //TODO: need to introduce a variable
                // that tracks whether the last command ended with |
                // otherwise can't distinguish echo good;
                // "echo good;"" vs. "echo bad |"
                // the latter should be fine, but must ask
                // for more characters
                job.pipelines.push_back(pipeline);
                pipeline.clear();
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
                partial_word.append(ParseDoubleQuote(job_str_copy, env));
                quote_word = true;
                continue;
            }
            case '\'': {
                partial_word.append(ParseSingleQuote(job_str_copy));
                quote_word = true;
                continue;
            }
            default : {
                partial_word.append(SwitchParsingTarget(matched, job_str_copy, env));
            }
        }
        debug("%s", "reloop");
    }

    //end of file, parse one last thing
    // char matched = job_str[match_index];
    // debug("end strcspn loc:%s %c", job_str_copy.c_str() + match_index, matched);
    // debug("end partial_word: %s", partial_word.c_str());
    // if (match_index != 0) partial_word.append(job_str_copy.substr(0,match_index));
    // debug("end partial_word: %s", partial_word.c_str());
    // if (partial_word.size() > 0) {
    //     if (next_word_redirects_in) {
    //         command.input_file = partial_word;
    //         next_word_redirects_in = false;
    //     } else if (next_word_redirects_out) {
    //         command.output_file = partial_word;
    //         next_word_redirects_out = false;
    //     } else command.words.push_back(partial_word);
    //     partial_word = string();
    // }
    // if (command.words.size() > 0) pipeline.commands.push_back(command);
    // TODO: if command size is 0, and we have a previous entry in this pipeline
    // the we must wait for more input
    //command breaking already handled, but must add pipeline to jobs still
    if (pipeline.commands.size() > 0) job.pipelines.push_back(pipeline);
    job.print();
    if (true) return job;
    //handle_found(loc[0], loc) -> switches to whichever function is appropriate to match


    // vector<string> pipeline_strs = StringUtil::Split(job_str, ";"); //TODO: fix, maybe escaped ('\")


    // for (string& pipeline_str : pipeline_strs) {
    //     ParsedPipeline pipeline;

    //     vector<string> command_strs = StringUtil::Split(pipeline_str, "|"); //TODO: fix, maybe escaped ('\")
    //     for (string& command_str : command_strs) {
    //         ParsedCommand command;
    //         // debug("%s", command_str.c_str());

    //         vector<string> split_command = StringUtil::Split(command_str, ">"); //TODO: fix, maybe escaped ('\")
    //         command_str = split_command[0];
    //         if (split_command.size() >= 2) {
    //             for (int i = 1; i < split_command.size(); i++) {
    //                 int start_pos = split_command[i].find_first_not_of(' ');
    //                 int end_pos = split_command[i].find_first_of(' ', start_pos);
    //                 command.output_file = split_command[i].substr(start_pos, end_pos);
    //                 if (end_pos < split_command[i].size()) {
    //                     command_str.push_back(' ');
    //                     command_str.append(split_command[i].substr(end_pos));
    //                 }
    //             }
    //         }

    //         split_command = StringUtil::Split(command_str, "<"); //TODO: fix, maybe escaped ('\")
    //         command_str = split_command[0];
    //         if (split_command.size() >= 2) {
    //             for (int i = 1; i < split_command.size(); i++) {
    //                 int start_pos = split_command[i].find_first_not_of(' ');
    //                 int end_pos = split_command[i].find_first_of(' ', start_pos);
    //                 command.input_file = split_command[i].substr(start_pos, end_pos);
    //                 if (end_pos < split_command[i].size()) {
    //                     command_str.push_back(' ');
    //                     command_str.append(split_command[i].substr(end_pos));
    //                 }
    //             }
    //         }

    //         // TODO: handle multiple whitespaces in a row, different whitespace
    //         //       types
    //         // TODO: handle input and output file redirection
    //         command.words = StringUtil::Split(command_str, " ");

    //         pipeline.commands.push_back(command);
    //     }

    //     job.pipelines.push_back(pipeline);
    // }

    // return job;
}


string JobParser::SwitchParsingTarget(char matched, string& job_str_copy, Environment& env) {
    switch(matched) {
        case '`': {
            return ParseBacktick(job_str_copy, env);
        }
        case '\\': {
            return ParseBackslash(job_str_copy);
        }
        case '$': {
            return ParseVariable(job_str_copy, env);
        }
        default : {
            throw IncompleteParseException("Matched Unknown character");
        }
    }

// return string(message);
}

string JobParser::ParseDoubleQuote(string& job_str_copy, Environment& env) {
    string quoted = string();
    int match_index;
    while((match_index = strcspn(job_str_copy.c_str(), "\"`$\\")) != job_str_copy.size()) {
        char matched = job_str_copy[match_index];
        debug("strcspn loc str:%s, char:%c", job_str_copy.c_str() + match_index, matched);
        quoted.append(job_str_copy.substr(0,match_index));
        job_str_copy = job_str_copy.substr(match_index + 1);
        if (matched == '\"') {
            return quoted;
        } else {
            quoted.append(SwitchParsingTarget(matched, job_str_copy, env));
        }
    }
    //TODO: this means unmatched ", so should do something different
    //for now
    throw IncompleteParseException("Incomplete job given, no valid closing quote (\")");
    // return quoted;

// return string(message);
}

string JobParser::ParseSingleQuote(string& job_str_copy) {
    // const char *loc = strpbrk(message, "\'");
    int match_index = strcspn(job_str_copy.c_str(), "\'");
    string quoted = job_str_copy.substr(0,match_index);
    debug("strcspn loc str:%s, char:\'", job_str_copy.c_str() + match_index);
    if (match_index != job_str_copy.size()) {
        job_str_copy = job_str_copy.substr(match_index + 1);
        return quoted;
    }
    //TODO: this means unmatched ', so should do something different
    //for now
    throw IncompleteParseException("Incomplete job given, no valid closing quote (')");
    // return quoted;

    //TODO: to handle "" case, instead of using size of previous to determine if should
    // append word, set some "valid word" thing that both operates when word is nonzero size
    // AND when we matched string.  Then will continue to match if no space, but will also
    // create word if necessary for "" or ''
// return string(message);
}

string JobParser::ParseBackslash(string& job_str_copy, char mode) {
    string quoted = job_str_copy.substr(0,1);
    if (mode == ' ') {
        job_str_copy = job_str_copy.substr(1);
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
        job_str_copy = job_str_copy.substr(1);
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
        job_str_copy = job_str_copy.substr(1);
        return quoted;
    }
    throw IncompleteParseException("Unknown backslash mode");

// return string(job_str_copy);
}

//TODO: parse whitespace on return IF AND ONLY IF this is in main unquoted lines
//    ---> ideally, in main thread, but then have a temp-state thing I don't love
string JobParser::ParseVariable(string& job_str_copy, Environment& env) { //TODO: push at front & reparse (may introduce words)
    char first_var_char = job_str_copy[0];
    string variable_name;
    if (string("*?#").find(first_var_char) != string::npos ||
        isdigit(first_var_char)) {
        variable_name = string(1, first_var_char);
        job_str_copy = job_str_copy.substr(1);
    } else if (first_var_char =='{') {
        int match_index = job_str_copy.find_first_of("}");
        variable_name = job_str_copy.substr(1,match_index-1); //skip first
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
    string tmp_var_str("VAR[");
    string close_str("]");
    tmp_var_str = tmp_var_str + variable_name + close_str;
    // return tmp_var_str;
    return var_value;
}

//TODO: parse whitespace on return IF AND ONLY IF this is in main unquoted lines
//    ---> ideally, in main thread, but then have a temp-state thing I don't love
string JobParser::ParseBacktick(string& job_str_copy, Environment& env) { //TODO: push at front & reparse (may introduce words)
    string quoted = string();
    int match_index;
    while((match_index = strcspn(job_str_copy.c_str(), "`\\")) != job_str_copy.size()) {
        char matched = job_str_copy[match_index];
        debug("strcspn loc str:%s, char:%c", job_str_copy.c_str() + match_index, matched);
        quoted.append(job_str_copy.substr(0,match_index));
        job_str_copy = job_str_copy.substr(match_index + 1);
        if (matched == '`') {
            string command_output_str;
            try {
                int fds[2];
                FileUtil::CreatePipe(fds);
                int read = fds[0];
                int write = fds[1];
                Job(quoted, env).RunAndWait(STDIN_FILENO, write);  //TODO redirect this to put inside our string
                // Job(quoted, env).RunAndWait(STDIN_FILENO, STDOUT_FILENO);  //TODO redirect this to put inside our string
                FileUtil::CloseDescriptor(write);
                command_output_str = FileUtil::DumpDescriptorIntoString(read);
                FileUtil::CloseDescriptor(read);

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
            // int extra_space = command_output_str.size();
            // printf("size of zdstroutput %lu %s\n", job_str_copy.size(), command_output_str.c_str());
            // job_str_copy.reserve(job_str_copy.capacity() + extra_space);
            // job_str_copy.insert(0, command_output_str);
            // job_str_copy = command_output_str + job_str_copy;
            // char *REMOVE = (char *)malloc(command_output_str.size() + job_str_copy.size() + 2);
            // memcpy(REMOVE, command_output_str.c_str(), command_output_str.size());
            // memcpy(REMOVE + command_output_str.size(), job_str_copy.c_str(), job_str_copy.size());
            // job_str_copy = REMOVE;

            job_str_copy = command_output_str + job_str_copy;
            debug("new string:%s\n", job_str_copy.c_str());
            // printf("size of nono %lu\n", job_str_copy.size());
            // printf("jobstr: %s\n\n", job_str_copy.c_str());
            //TODO: bash DOESN'T actually reparse from scratch, so we shouldn't
            //do this here
            return string();
        } else {
            quoted.append(ParseBackslash(job_str_copy, '`'));
        }
    }
    throw IncompleteParseException("Incomplete job given, no valid closing backtick (`)");
}


string JobParser::ParseTilde(string& job_str_copy, Environment& env) { //TODO: push at front & reparse (may introduce words)
    //if any previous character != space or similar, then ignore & return tilde
    //parse subsequent characters as username.
    //If find username, substitute that
    //else, tilde is literal & return that
    int match_index = job_str_copy.find_first_of("/\t\n ;|<>");
    string matched_str = job_str_copy.substr(0,match_index);
    if (matched_str.size() == 0) {
        //just expand tilde w/o username
        return env.get_variable("HOME"); //as per spec, this is default
    }
    string home_dir = FileUtil::GetUserHomeDirectory(matched_str);
    if (home_dir == "") {
        //no user found, just use literal tilde & consume no input
        return string("~");
    }
    job_str_copy = job_str_copy.substr(match_index);
    //CAN GET TO WORK: have "prev was space or ;|> (last of which will fail with ambigious
    // "/User/jakemck: Is a directory" [unclear if failed b/c redirecting to dir, or because
    //has standalone directory])
    return home_dir;

}





