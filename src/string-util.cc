#include "string-util.h"

vector<string> StringUtil::Split(const string &str, const string &delim) {
    vector<string> result;

    size_t pos = 0;
    size_t last_seen = 0;
    while ((pos = str.find(delim, last_seen)) != string::npos) {
        string token = str.substr(last_seen, pos - last_seen);
        result.push_back(token);
        last_seen = pos + delim.length();
    }
    result.push_back(str.substr(last_seen));

    return result;
}


// const vector<string> StringUtil::MatchNextInSet(const string &str, const string &delims) {
    // vector<string> result;

    // size_t pos = 0;
    // size_t last_seen = 0;
    // while ((pos = str.find(delim, last_seen)) != string::npos) {
    //     string token = str.substr(last_seen, pos - last_seen);
    //     result.push_back(token);
    //     last_seen = pos + delim.length();
    // }
    // result.push_back(str.substr(last_seen));

    // for 

    // return result;

    //             for (int i = 1; i < split_command.size(); i++) {
    //                 int start_pos = split_command[i].find_first_not_of(' ');
    //                 int end_pos = split_command[i].find_first_of(' ', start_pos);
    //                 command.outputFile = split_command[i].substr(start_pos, end_pos);
    //                 if (end_pos < split_command[i].size()) {
    //                     command_str.push_back(' ');
    //                     command_str.append(split_command[i].substr(end_pos));
    //                 }
    //             }
// }


vector<string> StringUtil::Split(const char * str, const string &delim) {
    string s(str);
    return Split(s, delim);
}


string StringUtil::PadRight(string const& str, size_t size) {
    if (str.size() < size) {
        return str + string(size - str.size(), ' ');
    } else {
        return str;
    }
}

string StringUtil::PadLeft(string const& str, size_t size) {
    if (str.size() < size) {
        return string(size - str.size(), ' ') + str;
    } else {
        return str;
    }
}

string StringUtil::Trim(std::string &str) {
    string s = TrimRight(str);
    s = TrimLeft(s);
    return s;
}

string StringUtil::TrimLeft(string &str) {
    string s = str;
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch) {
        return !isspace(ch);
    }));
    return s;
}

string StringUtil::TrimRight(string &str) {
    string s = str;
    s.erase(find_if(s.rbegin(), s.rend(), [](int ch) {
        return !isspace(ch);
    }).base(), s.end());
    return s;
}
