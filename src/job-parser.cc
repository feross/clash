#include "job-parser.h"
#include <cstring>

ParsedJob JobParser::Parse(string& job_str) {
    // const char* job_cstr = job_str.c_str();

    string job_str_copy(job_str);

    // vector<string> pipeline_strs;
    // vector<string> command_strs;
    //             ParsedCommand command;
    // int match_index;
    // while((match_index = strcspn(job_str_copy.c_str(), ";'`\"")) != job_str_copy.size()) {
    //     char matched = job_str[match_index];

    //     if (matched == ';') {
    //         if (match_index != 0) {
    //             if (job_str_copy[match_index-1] == '\\') continue;
    //         }
    //         command_strs.append(job_str_copy.substr(0, match_index))
    //         job_str_copy = job_str_copy.substr(match_index + 1);
    //     }
    // }
    // ParseVariable(job_str_copy);
    // ParseVariable(job_str_copy);
    // ParseVariable(job_str_copy);

    ParsedCommand command;
    ParsedPipeline pipeline;
    ParsedJob job;

    int match_index;
    string partial_word = string();
    bool next_word_redirects_out = false;
    bool next_word_redirects_in = false;
    while((match_index = strcspn(job_str_copy.c_str(), " \t\n;|<>$'`\"\\")) != job_str_copy.size()) {
        char matched = job_str_copy[match_index];
        printf("strcspn loc str:%s, char:%c\n", job_str_copy.c_str() + match_index, matched);
        printf("partial_word: %s\n", partial_word.c_str());
        if (match_index != 0) partial_word.append(job_str_copy.substr(0,match_index));
        printf("partial_word: %s\n", partial_word.c_str());
        job_str_copy = job_str_copy.substr(match_index + 1);
        printf("new job_str_copy:%s\n", job_str_copy.c_str());
        switch(matched) {

            case '\t':
            case ' ':
            case '\n': {
                printf("whitespace, prevstuf:%s\n", partial_word.c_str());
                if (partial_word.size() > 0) {
                    if (next_word_redirects_in) {
                        command.inputFile = partial_word;
                        next_word_redirects_in = false;
                    } else if (next_word_redirects_out) {
                        command.outputFile = partial_word;
                        next_word_redirects_out = false;
                    } else command.words.push_back(partial_word);
                    partial_word = string();
                }
                continue;
            }
            case ';': {
                if (partial_word.size() > 0) {
                    if (next_word_redirects_in) {
                        command.inputFile = partial_word;
                        next_word_redirects_in = false;
                    } else if (next_word_redirects_out) {
                        command.outputFile = partial_word;
                        next_word_redirects_out = false;
                    } else command.words.push_back(partial_word);
                    partial_word = string();
                }
                pipeline.commands.push_back(command);
                command.clear();
                job.pipelines.push_back(pipeline);
                pipeline.clear();
                continue;
            }
            case '|': {
                if (partial_word.size() > 0) {
                    if (next_word_redirects_in) {
                        command.inputFile = partial_word;
                        next_word_redirects_in = false;
                    } else if (next_word_redirects_out) {
                        command.outputFile = partial_word;
                        next_word_redirects_out = false;
                    } else command.words.push_back(partial_word);
                    partial_word = string();
                }
                pipeline.commands.push_back(command);
                command.clear();
                continue;
            }
            case '<': {
                if (partial_word.size() > 0) {
                    if (next_word_redirects_in) {
                        command.inputFile = partial_word;
                        next_word_redirects_in = false;
                    } else if (next_word_redirects_out) {
                        command.outputFile = partial_word;
                        next_word_redirects_out = false;
                    } else command.words.push_back(partial_word);
                    partial_word = string();
                }
                next_word_redirects_in = true;
                continue;
            }
            case '>': {
                if (partial_word.size() > 0) {
                    if (next_word_redirects_in) {
                        command.inputFile = partial_word;
                        next_word_redirects_in = false;
                    } else if (next_word_redirects_out) {
                        command.outputFile = partial_word;
                        next_word_redirects_out = false;
                    } else command.words.push_back(partial_word);
                    partial_word = string();
                }
                next_word_redirects_out = true;
                continue;
            }
            default : {
                partial_word.append(SwitchParsingTarget(matched, job_str_copy));
            }
        }
        printf("reloop\n");
    }
    //end of file, parse one last thing
    char matched = job_str[match_index];
    printf("end strcspn loc:%s %c\n", job_str_copy.c_str() + match_index, matched);
    printf("end partial_word: %s\n", partial_word.c_str());
    if (match_index != 0) partial_word.append(job_str_copy.substr(0,match_index));
    printf("end partial_word: %s\n", partial_word.c_str());
    if (partial_word.size() > 0) {
        if (next_word_redirects_in) {
            command.inputFile = partial_word;
            next_word_redirects_in = false;
        } else if (next_word_redirects_out) {
            command.outputFile = partial_word;
            next_word_redirects_out = false;
        } else command.words.push_back(partial_word);
        partial_word = string();
    }
    pipeline.commands.push_back(command);
    job.pipelines.push_back(pipeline);

    job.print();
    if (true) return job;
    //handle_found(loc[0], loc) -> switches to whichever function is appropriate to match


    vector<string> pipeline_strs = StringUtil::Split(job_str, ";"); //TODO: fix, maybe escaped ('\")


    for (string& pipeline_str : pipeline_strs) {
        ParsedPipeline pipeline;

        vector<string> command_strs = StringUtil::Split(pipeline_str, "|"); //TODO: fix, maybe escaped ('\")
        for (string& command_str : command_strs) {
            ParsedCommand command;
            // printf("%s\n", command_str.c_str());

            vector<string> split_command = StringUtil::Split(command_str, ">"); //TODO: fix, maybe escaped ('\")
            command_str = split_command[0];
            if (split_command.size() >= 2) {
                for (int i = 1; i < split_command.size(); i++) {
                    int start_pos = split_command[i].find_first_not_of(' ');
                    int end_pos = split_command[i].find_first_of(' ', start_pos);
                    command.outputFile = split_command[i].substr(start_pos, end_pos);
                    if (end_pos < split_command[i].size()) {
                        command_str.push_back(' ');
                        command_str.append(split_command[i].substr(end_pos));
                    }
                }
            }

            split_command = StringUtil::Split(command_str, "<"); //TODO: fix, maybe escaped ('\")
            command_str = split_command[0];
            if (split_command.size() >= 2) {
                for (int i = 1; i < split_command.size(); i++) {
                    int start_pos = split_command[i].find_first_not_of(' ');
                    int end_pos = split_command[i].find_first_of(' ', start_pos);
                    command.outputFile = split_command[i].substr(start_pos, end_pos);
                    if (end_pos < split_command[i].size()) {
                        command_str.push_back(' ');
                        command_str.append(split_command[i].substr(end_pos));
                    }
                }
            }

            // TODO: handle multiple whitespaces in a row, different whitespace
            //       types
            // TODO: handle input and output file redirection
            command.words = StringUtil::Split(command_str, " ");

            pipeline.commands.push_back(command);
        }

        job.pipelines.push_back(pipeline);
    }

    return job;
}


string JobParser::SwitchParsingTarget(char matched, string& message) {
    switch(matched) {
        case '\"': {
            return ParseDoubleQuote(message);
        }
        case '\'': {
            return ParseSingleQuote(message);
        }
        case '`': {
            return ParseBacktick(message);
        }
        case '\\': {
            return ParseBackslash(message);
        }
        case '$': {
            return ParseVariable(message);
        }
        default : {
            throw ParseException("Matched Unknown character");
        }
    }

return string(message);
}

string JobParser::ParseDoubleQuote(string& job_str_copy) {
    string quoted = string();
    int match_index;
    while((match_index = strcspn(job_str_copy.c_str(), "\"`$\\")) != job_str_copy.size()) {
        char matched = job_str_copy[match_index];
        printf("strcspn loc str:%s, char:%c", job_str_copy.c_str() + match_index, matched);
        quoted.append(job_str_copy.substr(0,match_index));
        job_str_copy = job_str_copy.substr(match_index + 1);
        if (matched == '\"') {
            return quoted;
        } else {
            quoted.append(SwitchParsingTarget(matched, job_str_copy));
        }
    }
    //TODO: this means unmatched ", so should do something different
    //for now
    return quoted;

// return string(message);
}

string JobParser::ParseSingleQuote(string& job_str_copy) {
    // const char *loc = strpbrk(message, "\'");
    int match_index = strcspn(job_str_copy.c_str(), "\'");
    printf("strcspn loc str:%s, char:\'", job_str_copy.c_str() + match_index);
    string quoted = job_str_copy.substr(0,match_index);
    job_str_copy = job_str_copy.substr(match_index + 1);
    return quoted;


// return string(message);
}

string JobParser::ParseBackslash(string& job_str_copy, char mode) {
    string quoted = job_str_copy.substr(0,1);
    job_str_copy = job_str_copy.substr(1);
    if (mode == ' ') {
        return quoted;
    }
    //"$", "`" (backquote), double-quote, backslash, or newline;
    if (mode == '\"') {
        string valid_matches("$`\"\\\n");
        if (valid_matches.find(quoted) == string::npos) {
            string unmodified("\\");
            unmodified.append(quoted);
            return unmodified;
        }
        return quoted;
    }
    //only backtick (inside, will parse as new job)
    if (mode == '`') {
        string valid_matches("`");
        if (valid_matches.find(quoted) == string::npos) {
            string unmodified("\\");
            unmodified.append(quoted);
            return unmodified;
        }
        return quoted;
    }
    throw ParseException("Unknown backslash mode");

// return string(job_str_copy);
}

string JobParser::ParseVariable(string& job_str_copy) { //TODO: push at front & reparse (may introduce words)
    printf("string:%s\n", job_str_copy.c_str());
    job_str_copy = job_str_copy.substr(1);
    printf("string:%s\n", job_str_copy.c_str());    
return string(job_str_copy);
}

string JobParser::ParseBacktick(string& job_str_copy) { //TODO: push at front & reparse (may introduce words)
    string quoted = string();
    int match_index;
    while((match_index = strcspn(job_str_copy.c_str(), "`\\")) != job_str_copy.size()) {
        char matched = job_str_copy[match_index];
        printf("strcspn loc str:%s, char:%c", job_str_copy.c_str() + match_index, matched);
        quoted.append(job_str_copy.substr(0,match_index));
        job_str_copy = job_str_copy.substr(match_index + 1);
        if (matched == '`') {
            string fake_return_str("COMMAND:[");
            fake_return_str.append(quoted);
            fake_return_str.append("]");
            int extra_space = fake_return_str.size();
            job_str_copy.reserve(job_str_copy.capacity() + extra_space);
            job_str_copy.insert(0, fake_return_str);
            return string();
        } else {
            quoted.append(ParseBackslash(job_str_copy, '`'));
        }
    }
    //TODO: this means unmatched ", so should do something different
    //for now
    return quoted;

// return string(job_str_copy);
}







