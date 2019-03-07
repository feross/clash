#include "util.h"

const vector<string> Util::StringSplit(const string &str, string delim) {
    vector<string> result;

    size_t last_seen = 0;
    size_t next = 0;
    for (; (next = str.find(delim, last_seen)) != string::npos; last_seen = next + 1) {
        result.push_back(str.substr(last_seen, next - last_seen));
    }
    result.push_back(str.substr(last_seen));

    return result;
}

string Util::PadRight(string const& str, size_t size) {
    if (str.size() < size) {
        return str + string(size - str.size(), ' ');
    } else {
        return str;
    }
}

string Util::PadLeft(string const& str, size_t size) {
    if (str.size() < size) {
        return string(size - str.size(), ' ') + str;
    } else {
        return str;
    }
}

// String trim implementation inspired by:
// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
void Util::LeftTrim(string &str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
void Util::RightTrim(string &str) {
    str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

// trim from both ends (in place)
void Util::Trim(std::string &str) {
    LeftTrim(str);
    RightTrim(str);
}

void Util::SafeClose(int fd) {
    if (close(fd) == -1) {
        warn("Error closing socket %d (%s)", fd, strerror(errno));
    }
}