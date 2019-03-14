/**
 * File descriptor utility functions.
 */

#pragma once

#include <algorithm>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <limits.h>
#include <pwd.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "log.h"
#include "string-util.h"

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
        /**
         * Creates a pipe, a unidirectional data channel that can be used for
         * interprocess communication and returns the two newly-allocated file
         * descriptors in a vector. The pipe is configured with O_CLOEXEC so it
         * will be automatically closed when execve() is called.
         *
         * Throws a FileException if the pipe cannot be created or configured
         * correctly.
         *
         * @return vector that contains exactly two file descriptors
         */
        static vector<int> CreatePipe();

        /**
         * Closes a file descriptor, so that it no longer refers to any file and
         * may be reused.
         *
         * Throws a FileException if the file cannnot be closed for any reason.
         *
         * @param fd Integer representing the file descriptor to close
         */
        static void CloseDescriptor(int fd);

        /**
         * Creates a copy of the file descriptor old_fd, using the descriptor
         * number specified in new_fd. If the file descriptor new_fd was
         * previously open, it is silently closed before being reused. After a
         * successful return, the old and new file descriptors may be used
         * interchangeably. They literally refer to the same open file
         * description.
         *
         * Throws a FileException if the file descriptor cannot be duplicated.
         *
         * @param new_fd The file descriptor to be closed and overwritten
         * @param old_fd The file descriptor to be duplicated
         */
        static void DuplicateDescriptor(int new_fd, int old_fd);

        /**
         * [ReadFileDescriptor description]
         * @param  fd [description]
         * @return            [description]
         */
        static string ReadFileDescriptor(int fd);

        /**
         * TODO
         *
         * Permission flags to enable user+group+global read, and user write.
         * In other words, these are the flags that would get set with
         * "chmod 644".
         */
        static int OpenFile(string& filePath, int flags = O_RDONLY,
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        static vector<string> GetDirectoryEntries(const string& path);
        static vector<string> GetGlobMatches(const string& pattern);
        static bool IsExecutableFile(const string& path);
        static bool IsDirectory(const string& path);

        // TODO: this does not belong here
        static pid_t CreateProcess();
        static void SetCurrentWorkingDirectory(const string& new_cwd);
        static string GetCurrentWorkingDirectory();
        static string GetUserHomeDirectory(const string& user);

    private:
        static bool GlobMatch(const string& pattern, const string& name);
};
