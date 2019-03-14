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
        static pid_t CreateProcess();
        static void SetCurrentWorkingDirectory(const string& new_cwd);
        static string GetCurrentWorkingDirectory();
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
