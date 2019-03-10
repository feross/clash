#include "environment.h"

extern char **environ;

Environment::Environment() {
    // Inherit environment variables from parent process
    int i = 0;
    char * var = environ[i];
    while (var != NULL) {
        vector<string> split = StringUtil::Split(var, "=");
        string name = split[0];
        string value = split[1];
        variables[name] = value;

        i += 1;
        var = environ[i];
    }

    // Ensure PATH always has a reasonable default
    if (!variables.count("PATH")) {
        variables["PATH"] = DEFAULT_PATH_VAR;
    }

    // current_working_directory = FileUtil::GetCurrentWorkingDirectory();

    // debug("Current working directory: %s", current_working_directory.c_str());
    // debug("%s", "Default environment: ");
    // for (auto const& [name, value] : variables) {
    //     debug("%s=%s", name.c_str(), value.c_str());
    // }
}

const string& Environment::get_variable(const string& name) {
    static const string default_value = "";

    if (variables.count(name)) {
        return variables[name];
    } else {
        return default_value;
    }
}

void Environment::set_variable(const string& name, const string& value) {
    variables[name] = value;
    debug("set variable '%s' to '%s'", name.c_str(), value.c_str());
}

void Environment::unset_variable(const string& name) {
    variables.erase(name);
    debug("unset variable '%s'", name.c_str());
}

// string Environment::get_current_working_directory() {
//     return current_working_directory;
// }

// void Environment::set_current_working_directory(string& new_cwd) {
//     FileUtil::SetCurrentWorkingDirectory(new_cwd);
//     current_working_directory = FileUtil::GetCurrentWorkingDirectory();
//     debug("set cwd to '%s'", current_working_directory.c_str());
// }
