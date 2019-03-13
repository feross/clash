#include "file-util.h"

// TODO: return a new vector of fds instead?
void FileUtil::CreatePipe(int fds[2]) {
    #ifdef _GNU_SOURCE
        if (pipe2(fds, O_CLOEXEC) == -1) {
            throw FileException("Failed to create pipe");
        }
    #else
        if (pipe(fds) == -1) {
            throw FileException("Failed to create pipe");
        }

        if (fcntl(fds[0], F_SETFD, FD_CLOEXEC) == -1 ||
            fcntl(fds[1], F_SETFD, FD_CLOEXEC) == -1) {
            throw FileException("Failed to configure pipe");
        }
    #endif
}

void FileUtil::CloseDescriptor(int fd) {
    if (close(fd) != 0) {
        throw FileException("Unable to close descriptor " + to_string(fd));
    }
}

void FileUtil::DuplicateDescriptor(int new_fd, int old_fd) {
    if (dup2(new_fd, old_fd) != old_fd) {
        throw FileException("Unable to duplicate descriptor " +
            to_string(new_fd));
    }
}

int FileUtil::OpenFile(string& filePath, int flags, mode_t mode) {
    int fd = open(filePath.c_str(), flags | O_CLOEXEC, mode);
    if (fd == -1) {
        throw FileException("Could not open \"" + filePath + "\"");
    }
    return fd;
}

string FileUtil::DumpDescriptorIntoString(int descriptor) {
    string contents = string();
    char buf[1024 + 1];
    int read_bytes;
    while ((read_bytes = read(descriptor, buf, 1024)) != 0) {
        buf[read_bytes] = '\0';
        contents.append(buf);
    }
    return contents;
}

vector<string> FileUtil::GetDirectoryEntries(const string& path) {
    vector<string> entries;

    DIR * dirp = opendir(path == "" ? "." : path.c_str());
    if (dirp == NULL) {
        throw FileException("Unable to open directory " + path);
    }

    while (true) {
        errno = 0;
        dirent * dir = readdir(dirp);

        // read error
        if (dir == NULL && errno != 0) {
            throw FileException(strerror(errno));
        }

        // end of stream
        if (dir == NULL && errno == 0) {
            break;
        }

        const char * entry = dir->d_name;

        if (strcmp(entry, ".") != 0 && strcmp(entry, "..") != 0) {
            entries.push_back(entry);
        }
    }

    if (closedir(dirp) != 0) {
        throw FileException("Unable to close directory " + path);
    }

    return entries;
}

// vector<string> FileUtil::GetGlobMatches(const string& glob_pattern) {
//     glob_t glob_matches;
//     memset(&glob_matches, 0, sizeof(glob_matches));

//     int ret = glob(glob_pattern.c_str(), 0, NULL, &glob_matches);
//     if(ret != 0 && ret != GLOB_NOMATCH) {
//         globfree(&glob_matches);
//         throw FileException("glob failed with return value" + to_string(ret));
//     }

//     vector<string> matches;
//     for(size_t i = 0; i < glob_matches.gl_pathc; i++) {
//         matches.push_back(string(glob_matches.gl_pathv[i]));
//     }

//     globfree(&glob_matches);
//     return matches;
// }

vector<string> FileUtil::GetGlobMatches(const string& glob_pattern) {
    debug("======================= GetGlobMatches(%s) =======================", glob_pattern.c_str());
    vector<string> current_matches;
    if (glob_pattern.length() == 0) {
        return current_matches;
    }

    vector<string> pattern_segments = StringUtil::Split(glob_pattern, "/");
    if (pattern_segments.front() == "") {
        pattern_segments.erase(pattern_segments.begin());
        current_matches.push_back("/");
    } else {
        current_matches.push_back("");
    }

    for (size_t i = 0; i < pattern_segments.size(); i++) {
        string pattern_segment = pattern_segments[i];
        debug("pattern_segment: %s", pattern_segment.c_str());

        vector<string> next_matches;
        for (string& current_match : current_matches) {
            debug("current_match: %s", current_match.c_str());

            vector<string> entries = GetDirectoryEntries(current_match);

            for (string& entry : entries) {
                // debug("entry: %s", entry.c_str());

                if (GlobMatch(pattern_segment, entry)) {
                    // debug("matched entry: %s", entry.c_str());
                    // TODO: if not on last iteration, ensure that entry is a
                    // directory, not a file
                    string next_match;
                    if (current_match == "") {
                        next_match = entry;
                    } else if (current_match == "/") {
                        next_match = "/" + entry;
                    } else {
                        next_match = current_match + "/" + entry;
                    }

                    // Ensure that files are only added when examining the last
                    // path segment. Otherwise, we skip them since they cannot
                    // possibly match when there are further segments to examine
                    // later.
                    if (IsDirectory(next_match) || i == pattern_segments.size() - 1) {
                        next_matches.push_back(next_match);
                    }
                }
            }
        }

        current_matches = next_matches;
    }

    return current_matches;
}

// Method for making star matching run in linear time was inspired by this
// blog post: https://research.swtch.com/glob
bool FileUtil::GlobMatch(const string& pattern, const string& name) {
    int px = 0;
    int nx = 0;
    int nextPx = 0;
    int nextNx = 0;

    vector<char> charClass;
    int inCharClass = false;
    bool inRange = false;
    char rangeStart = '\0';
    bool negateCharClass = false;

    while (px < pattern.length() || nx < name.length()) {
        if (px < pattern.length()) {
            char c = pattern[px];
            switch (c) {
                case '?': {
                    // single-character wildcard
                    if (nx < name.length()) {
                        px++;
                        nx++;
                        continue;
                    }
                    break;
                }
                case '*': {
                    // zero-or-more-character wildcard

                    // Try to match at nx. If that doesn't work out, restart at
                    // nx+1 next.
                    nextPx = px;
                    nextNx = nx + 1;
                    px++;
                    continue;
                }
                case '[': {
                    if (inCharClass) {
                        // Appears within character class, treat as literal
                        goto default_case;
                    }
                    inCharClass = true;
                    px++;
                    continue;
                }
                case ']': {
                    if (!inCharClass) {
                        // Appears outside of character class, treat as literal
                        goto default_case;
                    }
                    if (inRange) {
                        // TODO?
                    }
                    if (nx < name.length()) {
                        bool matched = find(charClass.begin(), charClass.end(),
                            name[nx]) != charClass.end();
                        if (matched != negateCharClass) {
                            px++;
                            nx++;
                            charClass.clear();
                            inCharClass = false;
                            negateCharClass = false;
                            continue;
                        }
                    }
                    break;
                }
                case '-': {
                    if (!inCharClass) {
                        // Appears outside of character class, treat as literal
                        goto default_case;
                    }
                    inRange = true;
                    rangeStart = charClass.back();
                    charClass.pop_back();
                    px++;
                    continue;
                }
                case '^': {
                    if (!inCharClass) {
                        // Appears outside of character class, treat as literal
                        goto default_case;
                    }
                    if (charClass.size() > 0) {
                        // Not first character of the character class (i.e. [a^b])
                        // so treat as literal
                        goto default_case;
                    }
                    negateCharClass = true;
                    px++;
                    continue;
                }
                default:
                default_case: {
                    if (inRange) {
                        char rangeEnd = c;
                        for (char ch = rangeStart; ch <= rangeEnd; ch++) {
                            charClass.push_back(ch);
                        }
                        px++;
                        inRange = false;
                        continue;
                    }
                    if (inCharClass) {
                        charClass.push_back(c);
                        px++;
                        continue;
                    }

                    // ordinary character
                    if (nx < name.length() && name[nx] == c) {
                        px++;
                        nx++;
                        continue;
                    }
                    break;
                }
            }
        }
        // Mismatch. Maybe restart.
        if (0 < nextNx && nextNx <= name.length()) {
            px = nextPx;
            nx = nextNx;
            charClass.clear();
            inCharClass = false;
            inRange = false;
            negateCharClass = false;
            continue;
        }
        return false;
    }
    // Matched all of pattern to all of name. Success.
    return true;
}

bool FileUtil::IsExecutableFile(const string& path) {
    return access(path.c_str(), X_OK) == 0;
}

bool FileUtil::IsDirectory(const string& path) {
    struct stat stat_result;
    if (stat(path.c_str(), &stat_result) != 0) {
        throw FileException(strerror(errno));
    }
    return stat_result.st_mode & S_IFDIR;
}

// TODO: this does not belong here
pid_t FileUtil::CreateProcess() {
    pid_t pid = fork();
    if (pid == -1) {
        throw FileException("Unable to create new process");
    }
    return pid;
}

void FileUtil::SetCurrentWorkingDirectory(const string& new_cwd) {
    if (chdir(new_cwd.c_str()) == -1) {
        throw FileException(new_cwd + ": No such file or directory");
    }
}

string FileUtil::GetCurrentWorkingDirectory() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        throw FileException(cwd + string(": No such file or directory"));
    }
    return string(cwd);
}

string FileUtil::GetUserHomeDirectory(const string& user) {
    passwd * pw = getpwnam(user.c_str());

    if (pw == NULL) {
        return "";
    }

    char * home_dir = pw->pw_dir;
    return string(home_dir);
}
