#pragma once

#include <map>
#include <set>
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

        void export_variable(const string& name);
        vector<string> get_export_variable_strings();
        // const set<string>& get_export_variables();

        // string get_current_working_directory();
        // void set_current_working_directory(string& new_cwd);
    private:
        map<string, string> variables;
        set<string> export_variables;
        // string current_working_directory;
};
