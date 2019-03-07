/**
 * Utility functions that don't quite fit anywhere else.
 */

#pragma once

#include <algorithm>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include "log.h"


using namespace std;

class Util {
    public:
        /**
         * Split the given string str into a vector of strings using the given
         * single-character delimiter delim.
         *
         * Example:
         *     StringSplit("12.34.56.78:9000", ":")
         *
         *     Returns a vector with two elements: "12.34.56.78", "9000"
         *
         * @param str The string to split
         * @param delim The delimiter to search for
         * @return Vector of result strings
         */
        static const vector<string> StringSplit(const string &str, string delim);

        /**
         * Add space to the right of the given string str so that it is at least
         * size characters wide.
         *
         * @param  str The string to pad
         * @param  size The size to ensure the string is padded to
         * @return The result string
         */
        static string PadRight(string const& str, size_t size);

        /**
         * Add space to the left of the given string str so that it is at least
         * size characters wide.
         *
         * @param  str The string to pad
         * @param  size The size to ensure the string is padded to
         * @return The result string
         */
        static string PadLeft(string const& str, size_t size);

        /**
         * Trim whitespace from the start of a string, passed by reference.
         * @param str String to "left trim"
         */
        static void LeftTrim(string &str);

        /**
         * Trim whitespace from the end of a string, passed by reference.
         * @param str String to "right trim"
         */
        static void RightTrim(string &str);

        /**
         * Trim whitespace from the start and end of a string, passed by
         * reference.
         * @param str String to trim
         */
        static void Trim(std::string &str);

        /**
         * Close a file descriptor and print a warning if closing fails.
         * @param fd The file descriptor to close
         */
        static void SafeClose(int fd);
};