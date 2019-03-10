#pragma once

#include <map>
#include <string>

#include "file-util.h"
#include "string-util.h"

using namespace std;

const string DEFAULT_PATH_VAR =
    "/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin";

class Environment {
    public:
        Environment();
        const string& get_variable(const string& name);
        void set_variable(const string& name, const string& value);
        void unset_variable(const string& name);

        // string get_current_working_directory();
        // void set_current_working_directory(string& new_cwd);
    private:
        map<string, string> variables;
        // string current_working_directory;
};
