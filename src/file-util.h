/**
 * File descriptor utility functions.
 */

#pragma once

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "log.h"

using namespace std;

class FileException : public exception {
    public:
        FileException(const string& message): message(message) {}
        FileException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
};

class FileUtil {
    public:
        static void CreatePipe(int fds[2]);
        static void CloseDescriptor(int fd);
        static void DuplicateDescriptor(int new_fd, int old_fd);
        static string DumpDescriptorIntoString(int descriptor);

        /**
         * TODO
         *
         * Permission flags to enable user+group+global read, and user write.
         * In other words, these are the flags that would get set with
         * "chmod 644".
         */
        static int OpenFile(string& filePath, int flags = O_RDONLY,
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        static vector<string> GetDirectoryEntries(string& path);

        // TODO: this does not belong here
        static pid_t CreateProcess();
        static void SetCurrentWorkingDirectory(const string& new_cwd);
        static string GetCurrentWorkingDirectory();
        static string GetUserHomeDirectory(string& user);
};
