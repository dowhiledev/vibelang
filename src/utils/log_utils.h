#ifndef VIBELANG_LOG_UTILS_H
#define VIBELANG_LOG_UTILS_H

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} log_level_t;

/* Initialize logging system */
void log_init(log_level_t min_level);

/* Set log level */
void log_set_level(log_level_t level);

/* Get current log level */
log_level_t log_get_level(void);

/* Log functions for different levels */
void log_debug(const char* format, ...);
void log_info(const char* format, ...);
void log_warning(const char* format, ...);
void log_error(const char* format, ...);
void log_fatal(const char* format, ...);

/* Log with source location info */
void log_debug_at(const char* file, int line, const char* format, ...);
void log_info_at(const char* file, int line, const char* format, ...);
void log_warning_at(const char* file, int line, const char* format, ...);
void log_error_at(const char* file, int line, const char* format, ...);
void log_fatal_at(const char* file, int line, const char* format, ...);

/* Convenience macros for source location */
#define DEBUG(...)   log_debug_at(__FILE__, __LINE__, __VA_ARGS__)
#define INFO(...)    log_info_at(__FILE__, __LINE__, __VA_ARGS__)
#define WARNING(...) log_warning_at(__FILE__, __LINE__, __VA_ARGS__)
#define ERROR(...)   log_error_at(__FILE__, __LINE__, __VA_ARGS__)
#define FATAL(...)   log_fatal_at(__FILE__, __LINE__, __VA_ARGS__)

#endif /* VIBELANG_LOG_UTILS_H */
