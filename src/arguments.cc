#include "arguments.h"

void Arguments::RegisterBool(string name, string description) {
    bool_args[name] = false;
    descriptions[name] = description;
}

void Arguments::RegisterInt(string name, string description) {
    int_args[name] = -1;
    descriptions[name] = description;
}

void Arguments::RegisterString(string name, string description) {
    string_args[name] = "";
    descriptions[name] = description;
}

void Arguments::RegisterAlias(string alias, string name) {
    alias_to_name[alias] = name;
    name_to_alias[name] = alias;
}

void Arguments::Parse(int argc, char* argv[]) {
    vector<string> arguments(argv + 1, argv + argc);
    for (int i = 0; i < arguments.size(); i++) {
        string arg = arguments[i];
        string name;

        // Named arguments are prefixed with two dashes (e.g. --help)
        string named_prefix("--");
        // Aliased arguments are prefixed with one dash (e.g. -h)
        string alias_prefix("-");

        if (arg.substr(0, named_prefix.size()) == named_prefix) {
            name = arg.substr(named_prefix.size());
        } else if (arg.substr(0, alias_prefix.size()) == alias_prefix) {
            string alias = arg.substr(alias_prefix.size());
            if (alias_to_name.count(alias)) {
                name = alias_to_name[alias];
            } else {
                throw ArgumentsException(arg + " option was unexpected");
            }
        }

        if (!name.empty()) {
            if (bool_args.count(name)) {
                bool_args[name] = true;
            } else if (int_args.count(name) || string_args.count(name)) {
                // Integer and string arguments consume the next token to
                // determine the argument value
                i += 1;
                if (i >= arguments.size()) {
                    throw ArgumentsException(arg + " option requires an argument");
                }
                string value = arguments[i];
                if (int_args.count(name)) {
                    int_args[name] = stoi(value);
                } else {
                    string_args[name] = value;
                 }
            } else {
                throw ArgumentsException(arg + " option was unexpected");
            }
        } else {
            // Unnamed arguments have no dash prefix
            unnamed_args.push_back(arg);
        }
    }
}

bool Arguments::get_bool(string name) {
    if (!bool_args.count(name)) {
        throw ArgumentsException("Missing argument " + name);
    }
    return bool_args[name];
}

int Arguments::get_int(string name) {
    if (!int_args.count(name)) {
        throw ArgumentsException("Missing argument " + name);
    }
    return int_args[name];
}

const string& Arguments::get_string(string name) {
    if (!string_args.count(name)) {
        throw ArgumentsException("Missing argument " + name);
    }
    return string_args[name];
}

const vector<string>& Arguments::get_unnamed() {
    return unnamed_args;
}

string Arguments::get_help_text() {
    string result;

    if (intro.size()) result += intro + "\n";
    result += "Usage:";

    unsigned long max_name_len = 0;
    unsigned long max_description_len = 0;
    for (auto const& [name, description] : descriptions) {
        max_name_len = max(max_name_len, name.size());
        max_description_len = max(max_description_len, description.size());
    }

    for (auto const& [name, description] : descriptions) {
        string type;
        if (bool_args.count(name)) {
            type = "bool";
        } else if (int_args.count(name)) {
            type = "int";
        } else {
            type = "string";
        }

        string alias;
        if (name_to_alias.count(name)) {
            alias = "-" + name_to_alias[name] + ", ";
        }

        result += "\n    ";
        result += StringUtil::PadRight(alias, 1);
        result += "--" + StringUtil::PadRight(name, max_name_len) + "  ";
        result += StringUtil::PadRight(description, max_description_len);
        result += " [" + type + "]";
    }
    return result;
}
