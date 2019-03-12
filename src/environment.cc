#include "environment.h"

extern char **environ;

Environment::Environment() {
    // Inherit environment variables from parent process
    int i = 0;
    for (char * var = environ[i]; var != NULL; i += 1, var = environ[i]) {
        vector<string> split = StringUtil::Split(var, "=");
        string name = split[0];
        string value = split[1];
        set_variable(name, value);
        export_variable(name);
    }


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
    // debug("set variable '%s' to '%s'", name.c_str(), value.c_str());
}

void Environment::unset_variable(const string& name) {
    variables.erase(name);
    export_variables.erase(name);
    debug("unset variable '%s'", name.c_str());
}

void Environment::export_variable(const string& name) {
    if (variables.count(name)) {
        export_variables.insert(name);
    }
}

vector<string> Environment::get_export_variable_strings() {
    vector<string> export_variable_strings;
    for (const string& name : export_variables) {
        export_variable_strings.push_back(name + "=" + get_variable(name));
    }
    return export_variable_strings;
}

string Environment::FindProgramPath(string& program_name)  {
    // If PATH is missing, use a reasonable default
    string path = variables.count("PATH")
        ? variables["PATH"]
        : DEFAULT_PATH_VAR;

    vector<string> search_paths = StringUtil::Split(path, ":");

    for (string search_path : search_paths) {
        vector<string> dirs = FileUtil::GetDirectoryEntries(search_path);
    }

    // If the program name contains a "/" character, then it is already a
    // path to an exutable so use it as-is.
    if (program_name.find("/") != string::npos) {
        return path;
    }

    // TODO
    return "";
}
