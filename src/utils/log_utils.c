#include "log_utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global variables for logging
static FILE *log_file = NULL;
static log_level_t current_log_level = LOG_LEVEL_INFO; // Default log level

// Initialize logging system
void init_logging(log_level_t level) {
  current_log_level = level;

  // Try to open log file in current directory first
  log_file = fopen("vibelang_debug.log", "a");

  // If that fails, try /tmp directory
  if (!log_file) {
    log_file = fopen("/tmp/vibelang_debug.log", "a");
  }

  // Log header if file was opened
  if (log_file) {
    time_t now = time(NULL);
    fprintf(log_file, "\n--- VibeLang Log Session Started at %s", ctime(&now));
    fflush(log_file);
  }
}

// Set log level
void set_log_level(log_level_t level) { current_log_level = level; }

// Set log file path
void set_log_file(const char *filepath) {
  // Close existing log file if open
  if (log_file) {
    fclose(log_file);
    log_file = NULL;
  }

  // Open new log file
  if (filepath) {
    log_file = fopen(filepath, "a");
  }
}

// Close logging system
void close_logging(void) {
  if (log_file) {
    time_t now = time(NULL);
    fprintf(log_file, "--- VibeLang Log Session Ended at %s\n", ctime(&now));
    fclose(log_file);
    log_file = NULL;
  }
}

// Debug level log
void log_debug(const char *format, ...) {
  if (current_log_level <= LOG_LEVEL_DEBUG) {
    va_list args;
    va_start(args, format);
    va_list args_copy;
    va_copy(args_copy, args);

    // Log to file if available
    if (log_file) {
      fprintf(log_file, "[DEBUG] ");
      vfprintf(log_file, format, args);
      fprintf(log_file, "\n");
      fflush(log_file);
    }

    // Also log to stdout for debug messages
    if (getenv("DEBUG_CONSOLE")) {
      printf("\033[36m[DEBUG] "); // Cyan color
      vprintf(format, args_copy);
      printf("\033[0m\n"); // Reset color
    }

    va_end(args);
    va_end(args_copy);
  }
}

// Info level log
void log_info(const char *format, ...) {
  if (current_log_level <= LOG_LEVEL_INFO) {
    va_list args;
    va_start(args, format);
    va_list args_copy;
    va_copy(args_copy, args);

    // Log to file if available
    if (log_file) {
      fprintf(log_file, "[INFO] ");
      vfprintf(log_file, format, args);
      fprintf(log_file, "\n");
      fflush(log_file);
    }

    // Also log to stdout for info messages
    printf("\033[32m[INFO] "); // Green color
    vprintf(format, args_copy);
    printf("\033[0m\n"); // Reset color

    va_end(args);
    va_end(args_copy);
  }
}

// Warning level log
void log_warn(const char *format, ...) {
  if (current_log_level <= LOG_LEVEL_WARN) {
    va_list args;
    va_start(args, format);
    va_list args_copy;
    va_copy(args_copy, args);

    // Log to file if available
    if (log_file) {
      fprintf(log_file, "[WARN] ");
      vfprintf(log_file, format, args);
      fprintf(log_file, "\n");
      fflush(log_file);
    }

    // Also log to stderr for warnings
    fprintf(stderr, "\033[33m[WARN] "); // Yellow color
    vfprintf(stderr, format, args_copy);
    fprintf(stderr, "\033[0m\n"); // Reset color

    va_end(args);
    va_end(args_copy);
  }
}

// Error level log
void log_error(const char *format, ...) {
  if (current_log_level <= LOG_LEVEL_ERROR) {
    va_list args;
    va_start(args, format);
    va_list args_copy;
    va_copy(args_copy, args);

    // Log to file if available
    if (log_file) {
      fprintf(log_file, "[ERROR] ");
      vfprintf(log_file, format, args);
      fprintf(log_file, "\n");
      fflush(log_file);
    }

    // Always log errors to stderr
    fprintf(stderr, "\033[31m[ERROR] "); // Red color
    vfprintf(stderr, format, args_copy);
    fprintf(stderr, "\033[0m\n"); // Reset color

    va_end(args);
    va_end(args_copy);
  }
}
