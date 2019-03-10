#include "environment.h"

extern char **environ;

Environment::Environment() {
    int i = 0;
    char * var = environ[i];
    while (var != NULL) {
        vector<string> split = StringUtil::Split(var, "=");
        string name = split[0];
        string value = split[1];
        vars[name] = value;

        i += 1;
        var = environ[i];
    }

    if (!vars.count("PATH")) {
        vars["PATH"] = DEFAULT_PATH_VAR;
    }

    debug("%s", "Default environment: ");
    for (auto const& [name, value] : vars) {
        debug("%s=%s", name.c_str(), value.c_str());
    }
}

string Environment::get_var(string& name) {
    if (vars.count(name)) {
        return vars[name];
    } else {
        return "";
    }
}

void Environment::set_var(string& name, string& value) {
    vars[name] = value;
}

string Environment::get_cwd() {
    return cwd;
}

void Environment::set_cwd(string& new_cwd) {
    cwd = new_cwd;
}
