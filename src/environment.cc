#include "environment.h"

extern char **environ;

Environment::Environment() {
    // Inherit environment variables from parent process
    int i = 0;
    for (char * var = environ[i]; var != NULL; i += 1, var = environ[i]) {
        vector<string> split = StringUtil::Split(var, "=");
        string name = split[0];
        string value = split[1];
        SetVariable(name, value);
        ExportVariable(name);
    }

    PopulatePathCache();
}

const string& Environment::GetVariable(const string& name) {
    static const string default_value = "";

    if (variables.count(name)) {
        return variables[name];
    } else {
        return default_value;
    }
}

void Environment::SetVariable(const string& name, const string& value) {
    variables[name] = value;
    if (name == "PATH") {
        PopulatePathCache();
    }
}

void Environment::UnsetVariable(const string& name) {
    variables.erase(name);
    export_variables.erase(name);
    if (name == "PATH") {
        PopulatePathCache();
    }
}

void Environment::ExportVariable(const string& name) {
    if (variables.count(name)) {
        export_variables.insert(name);
    }
}

vector<string> Environment::GetExportVariableStrings() {
    vector<string> export_variable_strings;
    for (const string& name : export_variables) {
        export_variable_strings.push_back(name + "=" + GetVariable(name));
    }
    return export_variable_strings;
}

// If no matching executable file can be found, return the first
// matching non-executable file. If nothing matched, empty string is
// returned.
string Environment::FindProgramPath(string& program_name)  {
    // If the program name contains a "/" character, then it is already a
    // path to an exutable so use it as-is.
    if (program_name.find("/") != string::npos) {
        return program_name;
    }

    // Fast path. If the program name is in the cache, return it.
    if (path_cache.count(program_name)) {
        return path_cache[program_name];
    }

    string first_program_path;

    // If PATH is missing, use a reasonable default
    string path = variables.count("PATH")
        ? variables["PATH"]
        : DEFAULT_PATH_VAR;

    vector<string> search_paths = StringUtil::Split(path, ":");

    for (string search_path : search_paths) {
        vector<string> entries = FileUtil::GetDirectoryEntries(search_path);
        for (string& entry : entries) {
            if (entry != program_name) {
                continue;
            }

            string program_path = search_path + "/" + program_name;
            if (first_program_path.empty()) {
                first_program_path = program_path;
            }

            if (FileUtil::IsExecutableFile(program_path)) {
                return program_path;
            }
        }
    }

    // May return empty string, indicating no file was matched
    return first_program_path;
}

void Environment::PopulatePathCache() {
    path_cache.clear();

    string path = variables.count("PATH")
        ? variables["PATH"]
        : DEFAULT_PATH_VAR;

    vector<string> search_paths = StringUtil::Split(path, ":");

    for (string search_path : search_paths) {
        vector<string> entries = FileUtil::GetDirectoryEntries(search_path);
        for (string& entry : entries) {
            if (!path_cache.count(entry)) {
                // Earlier path entries take priority over later ones, so
                // never overwrite a cache entry.
                string program_path = search_path + "/" + entry;
                path_cache[entry] = program_path;
            }
        }
    }
}
