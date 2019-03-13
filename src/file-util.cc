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

    DIR * dirp = opendir(path.c_str());
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
        entries.push_back(entry);
    }

    if (closedir(dirp) != 0) {
        throw FileException("Unable to close directory " + path);
    }

    return entries;
}

vector<string> FileUtil::GetGlobMatches(const string& glob_pattern) {
    glob_t glob_matches;
    memset(&glob_matches, 0, sizeof(glob_matches));

    int ret = glob(glob_pattern.c_str(), 0, NULL, &glob_matches);
    if(ret != 0 && ret != GLOB_NOMATCH) {
        globfree(&glob_matches);
        throw FileException("glob failed with return value" + to_string(ret));
    }

    vector<string> matches;
    for(size_t i = 0; i < glob_matches.gl_pathc; i++) {
        matches.push_back(string(glob_matches.gl_pathv[i]));
    }

    globfree(&glob_matches);
    return matches;
}

// vector<string> FileUtil::GetGlobMatches(const string& glob_pattern) {
//     vector<string> matches;
//     if (glob_pattern.length() == 0) {
//         return matches;
//     }

//     vector<string> dir_patterns = StringUtil::Split(glob_pattern, "/");
//     if (dir_patterns.front()[0] == "") {
//         dir_patterns.front()[0] = "/";
//     }

//     vector<string> entries = GetDirectoryEntries(".");
//     for (size_t i = 0; i < dir_patterns.size(); i++) {
//         string current_dir = dir_patterns[i];
//     }


//     return matches;
// }

// Linear-time globbing algorithm based on https://research.swtch.com/glob
// bool FileUtil::GlobMatch(string& pattern, string& name) {
//     int px = 0;
//     int nx = 0;
//     int nextPx = 0;
//     int nextNx = 0;
//     while (px < pattern.length() || nx < name.length()) {
//         if (px < pattern.length()) {
//             char c = pattern[px];
//             switch (c) {
//                 case '?': {
//                     // single-character wildcard
//                     if (nx < name.length()) {
//                         px++;
//                         nx++;
//                         continue;
//                     }
//                 }
//                 case '*': {
//                     // zero-or-more-character wildcard

//                     // Try to match at nx. If that doesn't work out, restart at
//                     // nx+1 next.
//                     nextPx = px;
//                     nextNx = nx + 1;
//                     px++;
//                     continue;
//                 }
//                 default: {
//                     // ordinary character
//                     if (nx < name.length() && name[nx] == c) {
//                         px++;
//                         nx++;
//                         continue;
//                     }
//                 }
//             }
//         }
//         // Mismatch. Maybe restart.
//         if (0 < nextNx && nextNx <= name.length()) {
//             px = nextPx;
//             nx = nextNx;
//             continue;
//         }
//         return false;
//     }
//     // Matched all of pattern to all of name. Success.
//     return true;
// }

bool FileUtil::IsExecutableFile(const string& path) {
    return access(path.c_str(), X_OK) == 0;
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
    // if (new_cwd.empty()) {
    //     new_cwd = GetUserHomeDirectory()
    // } //TODO: get current user directory if empty
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

// TODO: Use this in the parser for username parsing
string FileUtil::GetUserHomeDirectory(const string& user) {
    passwd * pw = getpwnam(user.c_str());

    if (pw == NULL) {
        return "";
    }

    char * home_dir = pw->pw_dir;
    return string(home_dir);
}
