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

        const string& GetVariable(const string& name);
        void SetVariable(const string& name, const string& value);
        void UnsetVariable(const string& name);

        void ExportVariable(const string& name);
        vector<string> GetExportVariableStrings();

        string FindProgramPath(string& program_name);
    private:
        void PopulatePathCache();
        map<string, string> variables;
        set<string> export_variables;
        map<string, string> path_cache;
};
