#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "log_utils.h"

static log_level_t current_log_level = LOG_LEVEL_INFO;
static const char* level_names[] = {
    "DEBUG", "INFO", "WARNING", "ERROR", "FATAL"
};
static const char* level_colors[] = {
    "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
static const char* reset_color = "\x1b[0m";

void log_init(log_level_t min_level) {
    current_log_level = min_level;
}

void log_set_level(log_level_t level) {
    current_log_level = level;
}

log_level_t log_get_level(void) {
    return current_log_level;
}

static void log_message(log_level_t level, const char* file, int line, const char* format, va_list args) {
    if (level < current_log_level) {
        return;
    }

    time_t now;
    struct tm* timeinfo;
    char timestamp[20];

    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    fprintf(stderr, "%s%s %s%s", level_colors[level], level_names[level], reset_color, timestamp);
    
    if (file) {
        fprintf(stderr, " %s:%d", file, line);
    }
    
    fprintf(stderr, ": ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    // Flush for immediate output
    fflush(stderr);
    
    // Exit on fatal error
    if (level == LOG_LEVEL_FATAL) {
        exit(EXIT_FAILURE);
    }
}

/* Standard log functions */
void log_debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_DEBUG, NULL, 0, format, args);
    va_end(args);
}

void log_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_INFO, NULL, 0, format, args);
    va_end(args);
}

void log_warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_WARNING, NULL, 0, format, args);
    va_end(args);
}

void log_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_ERROR, NULL, 0, format, args);
    va_end(args);
}

void log_fatal(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_FATAL, NULL, 0, format, args);
    va_end(args);
}

/* Log with source location */
void log_debug_at(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_DEBUG, file, line, format, args);
    va_end(args);
}

void log_info_at(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_INFO, file, line, format, args);
    va_end(args);
}

void log_warning_at(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_WARNING, file, line, format, args);
    va_end(args);
}

void log_error_at(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_ERROR, file, line, format, args);
    va_end(args);
}

void log_fatal_at(const char* file, int line, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_LEVEL_FATAL, file, line, format, args);
    va_end(args);
}
