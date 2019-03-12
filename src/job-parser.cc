// #include "job-parser.h"
// #include <cstring>
// #include <cctype>

// ParsedJob JobParser::Parse(string& job_str) {
//     string job_str_copy(job_str);
//     ParsedCommand command;
//     ParsedPipeline pipeline;
//     ParsedJob job;

//     // int match_index;
//     string partial_word = string();
//     bool next_word_redirects_out = false;
//     bool next_word_redirects_in = false;
//     bool quote_word = false;
//     //TODO: probably switch to "find_first_of" and "find_first_not_of" which do same thing I think
//     while(true) {
//         int match_index = strcspn(job_str_copy.c_str(), " \t\n;|<>$'`\"\\");
//         // debug("strcspn loc str:%s, char:%c", job_str_copy.c_str() + match_index, matched);
//         // debug("partial_word: %s", partial_word.c_str());
//         if (match_index != 0) partial_word.append(job_str_copy.substr(0,match_index));
//         // debug("partial_word: %s", partial_word.c_str());
//         // debug("new job_str_copy:%s", job_str_copy.c_str());
//         // string endword_chars("\t\n ;|<>");
//         if (match_index == job_str_copy.size() || string("\t\n ;|<>").find(job_str_copy[match_index])) {
//             //word break
//             if (partial_word.size() > 0 || quote_word) { //TODO: change to var, so works with "" words
//                 if (next_word_redirects_in) {
//                     command.input_file = partial_word;
//                     next_word_redirects_in = false;
//                 } else if (next_word_redirects_out) {
//                     command.output_file = partial_word;
//                     next_word_redirects_out = false;
//                 } else command.words.push_back(partial_word);
//                 partial_word = string();
//                 quote_word = false;
//             }
//         }
//         if (match_index == job_str_copy.size()) break;

//         char matched = job_str_copy[match_index];
//         job_str_copy = job_str_copy.substr(match_index + 1);

//         switch(matched) {
//             case '\t':
//             case ' ':
//             case '\n': {
//                 debug("whitespace, prevstuf:%s", partial_word.c_str());
//                 continue;
//             }
//             case ';': {
//                 pipeline.commands.push_back(command);
//                 command.clear();
//                 job.pipelines.push_back(pipeline);
//                 pipeline.clear();
//                 continue;
//             }
//             case '|': {
//                 pipeline.commands.push_back(command);
//                 command.clear();
//                 continue;
//             }
//             case '<': {
//                 next_word_redirects_in = true;
//                 continue;
//             }
//             case '>': {
//                 next_word_redirects_out = true;
//                 continue;
//             }
//             case '\"': {
//                 partial_word.append(ParseDoubleQuote(job_str_copy));
//                 continue;
//             }
//             case '\'': {
//                 partial_word.append(ParseSingleQuote(job_str_copy));
//                 quote_word = true;
//                 continue;
//             }
//             default : {
//                 partial_word.append(SwitchParsingTarget(matched, job_str_copy));
//                 quote_word = true;
//             }
//         }
//         debug("%s", "reloop");
//     }
//     //end of file, parse one last thing
//     // char matched = job_str[match_index];
//     // debug("end strcspn loc:%s %c", job_str_copy.c_str() + match_index, matched);
//     // debug("end partial_word: %s", partial_word.c_str());
//     // if (match_index != 0) partial_word.append(job_str_copy.substr(0,match_index));
//     // debug("end partial_word: %s", partial_word.c_str());
//     // if (partial_word.size() > 0) {
//     //     if (next_word_redirects_in) {
//     //         command.input_file = partial_word;
//     //         next_word_redirects_in = false;
//     //     } else if (next_word_redirects_out) {
//     //         command.output_file = partial_word;
//     //         next_word_redirects_out = false;
//     //     } else command.words.push_back(partial_word);
//     //     partial_word = string();
//     // }
//     if (command.words.size() > 0) pipeline.commands.push_back(command);
//     if (pipeline.commands.size() > 0) job.pipelines.push_back(pipeline);

//     job.print();
//     if (true) return job;
//     //handle_found(loc[0], loc) -> switches to whichever function is appropriate to match


//     // vector<string> pipeline_strs = StringUtil::Split(job_str, ";"); //TODO: fix, maybe escaped ('\")


//     // for (string& pipeline_str : pipeline_strs) {
//     //     ParsedPipeline pipeline;

//     //     vector<string> command_strs = StringUtil::Split(pipeline_str, "|"); //TODO: fix, maybe escaped ('\")
//     //     for (string& command_str : command_strs) {
//     //         ParsedCommand command;
//     //         // debug("%s", command_str.c_str());

//     //         vector<string> split_command = StringUtil::Split(command_str, ">"); //TODO: fix, maybe escaped ('\")
//     //         command_str = split_command[0];
//     //         if (split_command.size() >= 2) {
//     //             for (int i = 1; i < split_command.size(); i++) {
//     //                 int start_pos = split_command[i].find_first_not_of(' ');
//     //                 int end_pos = split_command[i].find_first_of(' ', start_pos);
//     //                 command.output_file = split_command[i].substr(start_pos, end_pos);
//     //                 if (end_pos < split_command[i].size()) {
//     //                     command_str.push_back(' ');
//     //                     command_str.append(split_command[i].substr(end_pos));
//     //                 }
//     //             }
//     //         }

//     //         split_command = StringUtil::Split(command_str, "<"); //TODO: fix, maybe escaped ('\")
//     //         command_str = split_command[0];
//     //         if (split_command.size() >= 2) {
//     //             for (int i = 1; i < split_command.size(); i++) {
//     //                 int start_pos = split_command[i].find_first_not_of(' ');
//     //                 int end_pos = split_command[i].find_first_of(' ', start_pos);
//     //                 command.input_file = split_command[i].substr(start_pos, end_pos);
//     //                 if (end_pos < split_command[i].size()) {
//     //                     command_str.push_back(' ');
//     //                     command_str.append(split_command[i].substr(end_pos));
//     //                 }
//     //             }
//     //         }

//     //         // TODO: handle multiple whitespaces in a row, different whitespace
//     //         //       types
//     //         // TODO: handle input and output file redirection
//     //         command.words = StringUtil::Split(command_str, " ");

//     //         pipeline.commands.push_back(command);
//     //     }

//     //     job.pipelines.push_back(pipeline);
//     // }

//     // return job;
// }


// string JobParser::SwitchParsingTarget(char matched, string& job_str_copy) {
//     switch(matched) {
//         case '`': {
//             return ParseBacktick(job_str_copy);
//         }
//         case '\\': {
//             return ParseBackslash(job_str_copy);
//         }
//         case '$': {
//             return ParseVariable(job_str_copy);
//         }
//         default : {
//             throw ParseException("Matched Unknown character");
//         }
//     }

// // return string(message);
// }

// string JobParser::ParseDoubleQuote(string& job_str_copy) {
//     string quoted = string();
//     int match_index;
//     while((match_index = strcspn(job_str_copy.c_str(), "\"`$\\")) != job_str_copy.size()) {
//         char matched = job_str_copy[match_index];
//         debug("strcspn loc str:%s, char:%c", job_str_copy.c_str() + match_index, matched);
//         quoted.append(job_str_copy.substr(0,match_index));
//         job_str_copy = job_str_copy.substr(match_index + 1);
//         if (matched == '\"') {
//             return quoted;
//         } else {
//             quoted.append(SwitchParsingTarget(matched, job_str_copy));
//         }
//     }
//     //TODO: this means unmatched ", so should do something different
//     //for now
//     return quoted;

// // return string(message);
// }

// string JobParser::ParseSingleQuote(string& job_str_copy) {
//     // const char *loc = strpbrk(message, "\'");
//     int match_index = strcspn(job_str_copy.c_str(), "\'");
//     string quoted = job_str_copy.substr(0,match_index);
//     debug("strcspn loc str:%s, char:\'", job_str_copy.c_str() + match_index);
//     if (match_index == job_str_copy.size()) {
//         job_str_copy = job_str_copy.substr(match_index + 1);
//         return quoted;
//     }
//     //TODO: this means unmatched ', so should do something different
//     //for now
//     return quoted;

//     //TODO: to handle "" case, instead of using size of previous to determine if should
//     // append word, set some "valid word" thing that both operates when word is nonzero size
//     // AND when we matched string.  Then will continue to match if no space, but will also
//     // create word if necessary for "" or ''
// // return string(message);
// }

// string JobParser::ParseBackslash(string& job_str_copy, char mode) {
//     string quoted = job_str_copy.substr(0,1);
//     if (mode == ' ') {
//         job_str_copy = job_str_copy.substr(1);
//         return quoted;
//     }
//     //"$", "`" (backquote), double-quote, backslash, or newline;
//     if (mode == '\"') {
//         string valid_matches("$`\"\\\n");
//         if (valid_matches.find(quoted) == string::npos) {
//             string unmodified("\\");
//             // unmodified.append(quoted);
//             return unmodified;
//         }
//         job_str_copy = job_str_copy.substr(1);
//         return quoted;
//     }
//     //only backtick (inside, will parse as new job)
//     if (mode == '`') {
//         string valid_matches("`");
//         if (valid_matches.find(quoted) == string::npos) {
//             string unmodified("\\");
//             //newer TODO: bash actually seems to do the old behavior - e.g. echo `\\` is a complete command according to bash
//             // -> so should actually allow escaping of \ by \.
//             // unmodified.append(quoted); //TODO: WRONG to leave in, I think - e.g. double backslash, shouldn't consume second backslack
//             //TODO: confirm this was right to comment out (i.e. instead output single
//             // backslash & leave next char inside the job_str_copy in case it's special
//             // job_str_copy = job_str_copy.substr(1); //tested - bash does this, not what the spec says
//             return unmodified;
//         }
//         //TODO: bash actually behaves incorrectly/differently than ousterhout's code
//         //namely, bash will consider something like echo `echo \\` a complete command, even
//         //though according to spec that first backslashs should be ignored while scanning for end
//         //and the second one should cause the last ` not to match
//         job_str_copy = job_str_copy.substr(1);
//         return quoted;
//     }
//     throw ParseException("Unknown backslash mode");

// // return string(job_str_copy);
// }

// string JobParser::ParseVariable(string& job_str_copy) { //TODO: push at front & reparse (may introduce words)
//     if (isdigit(job_str_copy[0])) {
//         // string tmp_var_str("VAR[" + job_str_copy[0] + "]");
//         string tmp_var_str("VAR[");
//         string close_str("]");
//         tmp_var_str = tmp_var_str + job_str_copy[0] + close_str;
//         //TODO: lookup correct variable
//         job_str_copy = job_str_copy.substr(1);
//         return tmp_var_str;
//     } else if (isalpha(job_str_copy[0])) {
//         int match_index = job_str_copy.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
//         string matched_str = job_str_copy.substr(0,match_index);
//         string tmp_var_str("VAR[");
//         string close_str("]");
//         tmp_var_str = tmp_var_str + matched_str + close_str;
//         job_str_copy = job_str_copy.substr(match_index);
//         return tmp_var_str;
//     } else if (job_str_copy[0] == '{') {
//         int match_index = job_str_copy.find_first_of("}");
//         string matched_str = job_str_copy.substr(1,match_index-1); //skip first
//         string tmp_var_str("VAR[");
//         string close_str("]");
//         tmp_var_str = tmp_var_str + matched_str + close_str;
//         job_str_copy = job_str_copy.substr(match_index + 1);
//         return tmp_var_str;
//     } else { //no match, output literal $
//         string unmodified("$");
//         return unmodified;
//     }
//     //if { is first char, need to match to end }
//     //else if number, *, #, or ? is first character, that's all we match
//     //else if letter, match until first non-letter, non-number character)

//     //Then must replace as approriate... all should be in environ, right?
// //     debug("string:%s", job_str_copy.c_str());
// //     job_str_copy = job_str_copy.substr(1);
// //     debug("string:%s", job_str_copy.c_str());
// // return string(job_str_copy);
// }

// string JobParser::ParseBacktick(string& job_str_copy) { //TODO: push at front & reparse (may introduce words)
//     string quoted = string();
//     int match_index;
//     while((match_index = strcspn(job_str_copy.c_str(), "`\\")) != job_str_copy.size()) {
//         char matched = job_str_copy[match_index];
//         debug("strcspn loc str:%s, char:%c", job_str_copy.c_str() + match_index, matched);
//         quoted.append(job_str_copy.substr(0,match_index));
//         job_str_copy = job_str_copy.substr(match_index + 1);
//         if (matched == '`') {
//             string fake_return_str("COMMAND:[");
//             fake_return_str.append(quoted);
//             fake_return_str.append("]");
//             int extra_space = fake_return_str.size();
//             job_str_copy.reserve(job_str_copy.capacity() + extra_space);
//             job_str_copy.insert(0, fake_return_str);
//             return string();
//         } else {
//             quoted.append(ParseBackslash(job_str_copy, '`'));
//         }
//     }
//     //TODO: this means unmatched ", so should do something different
//     //for now
//     return quoted;

// // return string(job_str_copy);
// }


// string JobParser::ParseTilde(string& job_str_copy) { //TODO: push at front & reparse (may introduce words)
//     //if any previous character != space or similar, then ignore & return tilde
//     //parse subsequent characters as username.
//     //If find username, substitute that
//     //else, tilde is literal & return that


//     //CAN GET TO WORK: have "prev was space or ;|> (last of which will fail with ambigious
//     // "/User/jakemck: Is a directory" [unclear if failed b/c redirecting to dir, or because
//     //has standalone directory])


//     debug("string:%s", job_str_copy.c_str());
//     job_str_copy = job_str_copy.substr(1);
//     debug("string:%s", job_str_copy.c_str());
// return string(job_str_copy);

// }





