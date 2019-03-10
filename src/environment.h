#pragma once

#include <map>
#include <string>

#include "string-util.h"

using namespace std;

const string DEFAULT_PATH_VAR =
    "/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin";

class Environment {
    public:
        Environment();
        string get_var(string& name);
        void set_var(string& name, string& value);

        string get_cwd();
        void set_cwd(string& new_cwd);
    private:
        map<string, string> vars;
        string cwd;
};
