/**
 * Process utility functions.
 */

#pragma once

#include <limits.h>
#include <pwd.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

class ProcUtil {
    public:
        /**
         * Creates a new process by duplicating the calling process. The new
         * process is referred to as the child process. The calling process is
         * referred to as the parent process. The child process and the parent
         * process run in separate memory spaces.
         *
         * This function effective "returns" twice, once in the parent process
         * and once again in the child process. In the parent process, the
         * return value is the pid of the child process. In the child process,
         * the return value is the sentinel value 0 which indicates that
         * execution is in the child process.
         *
         * Throws a ProcException if the new process cannot be created.
         *
         * @return pid of the child process, or zero if in the child process
         */
        static pid_t CreateProcess();

        /**
         * Changes the current working directory of this process to the
         * directory specified in new_cwd.
         *
         * Throws a ProcException if the current working directory cannot be
         * changed.
         *
         * @param new_cwd The directory to change the current working directory
         *        to.
         */
        static void SetCurrentWorkingDirectory(const string& new_cwd);

        /**
         * Get the current working directory of this process as a string.
         * Throws a ProcException if the current working directory cannot be
         * retrieved.
         *
         * @return String representing the current working directory of the
         *         current running process.
         */
        static string GetCurrentWorkingDirectory();

        /**
         * Get the home direcotry of the given user. If the given user does not
         * exist on the system, then an empty string is returned instead of
         * a home directory path..
         *
         * @param  user The login username of the user whose home directory will
         *              be looked up.
         * @return      A string representing a home directory path, or an empty
         *              string if the user does not exist on the system.
         */
        static string GetUserHomeDirectory(const string& user);
};

class ProcException : public exception {
    public:
        ProcException(const string& message): message(message) {}
        ProcException(const char* message): message(message) {}
        const char* what() const noexcept { return message.c_str(); }
    private:
        string message;
};
