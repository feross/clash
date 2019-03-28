/**
 * A logger designed to standardize logging across an entire project, ensures
 * that logging is thread-safe, and offers support to different "log levels".
 *
 * Users of this logger should set the desired logging level using the extern
 * LOG_LEVEL variable. All log messages are categorized into one of four levels,
 * so only the desired logs can be included when running a program.
 *
 * Log levels, from most verbose to least verbose:
 *     - DEBUG (verbose, protocol information, variable values, etc.)
 *     - INFO  (important user-facing information)
 *     - WARN  (non-fatal warnings)
 *     - ERROR (fatal errors)
 *
 * Example:
 *     LogType LOG_LEVEL = INFO; // Exclude logs at DEBUG level
 *
 *     int main(int argc, char* argv[]) {
 *         debug("%s", "This is a debug message"); // Not printed!
 *         info("%s", "This is some useful information");
 *         warn("%s", "This is a warning!");
 *         error("Unexpected error: %s", error_message);
 *     }
 */

#pragma once

#include <cstdio>
#include <mutex>

enum LogType { DEBUG, INFO, WARN, ERROR };
static const std::string LogTypeStrings[] = { "DEBUG", "INFO", "WARN", "ERROR" };
static std::mutex log_mutex;

extern LogType LOG_LEVEL;

/**
 * Print a debug message, accepting printf-style arguments.
 *
 * @param format C string that contains the text to be written.
 * @param ... (additional arguments)
 */
template<typename... Args>
void debug(const char* format, Args... args) {
    log(DEBUG, format, args...);
}

/**
 * Print an informational message, accepting printf-style arguments.
 *
 * @param format C string that contains the text to be written.
 * @param ... (additional arguments)
 */
template<typename... Args>
void info(const char* format, Args... args) {
    log(INFO, format, args...);
}

/**
 * Print an informational message, accepting printf-style arguments.
 *
 * @param format C string that contains the text to be written.
 * @param ... (additional arguments)
 */
template<typename... Args>
void warn(const char* format, Args... args) {
    log(WARN, format, args...);
}

/**
 * Print an informational message, accepting printf-style arguments.
 *
 * @param format C string that contains the text to be written.
 * @param ... (additional arguments)
 */
template<typename... Args>
void error(const char* format, Args... args) {
    log(ERROR, format, args...);
}

/**
 * Internal function. Print to stdout using the given log level, in a
 * thread-safe manner.
 */
template<typename... Args>
static void log(LogType level, const char* format, Args... args) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (level < LOG_LEVEL) return;
    printf("[%s] ", LogTypeStrings[level].c_str());
    printf(format, args...);
    printf("\n");
}