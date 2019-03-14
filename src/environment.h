/**
 * The local shell environment. Provides an abstraction over the concept of
 * environment variables and looks up program names efficiently using the
 * current PATH environment variable, using an internal cache whenever possible.
 */

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
        /**
         * Create a new shell environment. By default, this environment inherits
         * environment variables from the parent process, exports them all, and
         * warms the PATH cache using the default PATH inherited from the parent
         * process, or if there was no PATH variable set in parent process then
         * a reasonable default is used.
         */
        Environment();

        /**
         * Get the environment variable with the given name. If no variable with
         * the given name exists, then an empty string is returned.
         *
         * @param  name The environment variable name
         * @return      The current value of the environment variable
         */
        const string& GetVariable(const string& name);

        /**
         * Set the environment variable with the given name to the given value.
         * If the variable name is "PATH", then the internal PATH cache is
         * re-populated.
         *
         * @param name  The environment variable name
         * @param value The new value for the environment variable
         */
        void SetVariable(const string& name, const string& value);

        /**
         * Delete the environment variable with the given name. If the variable
         * name is "PATH", then the internal PATH cache is re-populated with
         * a reasonable default value.
         *
         * @param name The environment variable name
         */
        void UnsetVariable(const string& name);

        /**
         * Mark the variable with the given name to be exported. This means that
         * subprocesses started from this environment should contain this
         * variable.
         *
         * @param name The environment variable name
         */
        void ExportVariable(const string& name);

        /**
         * Return a vector of environment variable names and values, where each
         * entry in the vector represents a single name and value pair,
         * separated by an equals sign.
         *
         * Example: ["PATH=/bin:/usr/bin:/usr/local/bin","HOME=/home/user"]
         *
         * @return Vector of environment variable strings
         */
        vector<string> GetExportVariableStrings();

        /**
         * Looks for a program with the given program_name in the locations
         * listed in the PATH environment variable. Returns an absolute path to
         * the given program. If no suitable executable file could be found,
         * then the first matching non-executable file is returned, or an empty
         * string if no matching programs could be found.
         *
         * The search process is very fast in most cases since an internal cache
         * of all the binaries included in all the directories in the PATH is
         * created at initialization time and anytime the PATH variable changes.
         * However, if the contents of any of the directories in the PATH
         * changes without the PATH variable value changing, then the new
         * binaries will not be in the cache.
         *
         * If the given program_name is not in the cache, it will still be found
         * because the PATH is always searched directly when there is a cache
         * miss.
         *
         * @param  program_name The program name to search for.
         * @return              An absolute path to the given program, or an
         *                      empty string if no matching program could be
         *                      found.
         */
        string FindProgramPath(string& program_name);
    private:
        /**
         * Clear the contents of the PATH cache and rebuild it from scratch
         * using the latest contents of the directories included in the PATH
         * environment variable.
         */
        void PopulatePathCache();

        /**
         * Map of environment variable name to value.
         */
        map<string, string> variables;

        /**
         * Set of variable names which are marked as exported.
         */
        set<string> export_variables;

        /**
         * The PATH cache. Map of program names to absolute paths where the
         * given programs are located.
         */
        map<string, string> path_cache;
};
