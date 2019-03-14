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
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "log.h"
#include "string-util.h"

using namespace std;

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
         * Reads the complete contents of the given file descriptor fd and
         * returns it as a string.
         *
         * @param  fd The file descriptor to read from
         * @return File contents as a string
         */
        static string ReadFileDescriptor(int fd);

        /**
         * Open the file specified by file_path. The return value is a file
         * descriptor, a small, nonnegative integer that is used in subsequent
         * system calls to refer to the open file. The file descriptor returned
         * by a successful call will be the lowest-numbered file descriptor not
         * currently open for the process.
         *
         * Optionally, flags and a mode can be specified which change the
         * behavior of the opened file, e.g. opening it for reading vs. writing
         * or setting the permissions on a newly-created file. See the standard
         * C documentation for open() to learn more about these options.
         *
         * Throws a FileException if the file cannot be opened.
         *
         * @param file_path Filesystem path to the file to open
         * @param flags Standard open() flags to configure the file descriptor
         * @param mode Standard open() flags to configure the file's mode
         * @return The newly-opened file descriptor
         */
        static int OpenFile(string& file_path, int flags = O_RDONLY,
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        /**
         * Return a vector of all directory entries in the given directory path.
         * Skips over trivial directory entries like "." and "..".
         *
         * Throws a FileException if the directory cannot be opened or the
         * stream of directory entries cannot be read.
         *
         * @param path Filesystem path to the directory to read entries from
         * @return Vector of strings representing directory entries
         */
        static vector<string> GetDirectoryEntries(const string& path);

        /**
         * Get all files or diretories which match the given "glob" pattern.
         * Glob patterns are quite similar to regular expressions but they are
         * not quite the same. The rules for glob matching are too complex to
         * list here, but complete information can be found online from various
         * resources such as http://tldp.org/LDP/abs/html/globbingref.html.
         *
         * The search starts in the current working directory. Throws a
         * FileException if the given pattern is invalid, e.g. it has
         * unclosed character class.
         *
         * Returns a vector of all the files or directories which match the
         * given glob pattern. If no files or directories are matched, then
         * vector containing a single string representing the original pattern
         * is returned.
         *
         * @param pattern The glob pattern to match against
         * @return The vector of strings that represent matched files and directories
         */
        static vector<string> GetGlobMatches(const string& pattern);

        /**
         * Returns true if the given file or directory has the executable
         * permission bit set.
         *
         * @param Filesystem path to the file to check
         * @return True if file is executable
         */
        static bool IsExecutableFile(const string& path);

        /**
         * Returns true if the given path represents a file (as opposed to a
         * directory).

         * @param  path Filesystem path to the entry to check
         * @return True if the given path represents a file
         */
        static bool IsDirectory(const string& path);

    private:
        static bool GlobMatch(const string& pattern, const string& name);
};

class FileException : public exception {
    public:
        FileException(const string& message): message(message) {}
        FileException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
};
