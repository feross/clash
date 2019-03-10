#include "string-util.h"

const vector<string> StringUtil::Split(const string &str, const string &delim) {
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

void StringUtil::Trim(std::string &str) {
    TrimLeft(str);
    TrimRight(str);
}

void StringUtil::TrimLeft(string &str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

void StringUtil::TrimRight(string &str) {
    str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

