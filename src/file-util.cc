#include "file-util.h"

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

vector<string> FileUtil::GetDirectoryEntries(string& path) {
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

        entries.push_back(dir->d_name);
        debug("found directory: %s\n", dir->d_name);
    }

    if (closedir(dirp) != 0) {
        throw FileException("Unable to close directory " + path);
    }

    return entries;
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
        throw FileException("Unable to set current working directory " +
            new_cwd);
    }
}

string FileUtil::GetCurrentWorkingDirectory() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        throw FileException("Unable to get current working directory");
    }
    return string(cwd);
}

// TODO: Use this in the parser for username parsing
string FileUtil::GetUserHomeDirectory(string& user) {
    passwd * pw = getpwnam(user.c_str());

    if (pw == NULL) {
        return "";
    }

    char * home_dir = pw->pw_dir;
    return string(home_dir);
}

