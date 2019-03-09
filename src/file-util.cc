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

// TODO: this does not belong here
pid_t FileUtil::CreateProcess() {
    pid_t pid = fork();
    if (pid == -1) {
        throw FileException("Unable to create new process");
    }
    return pid;
}