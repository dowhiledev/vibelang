#ifndef LOG_UTILS_H
#define LOG_UTILS_H

// Define log levels
typedef enum {
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR
} log_level_t;

// Log macros
#define DEBUG(format, ...) log_debug(format, ##__VA_ARGS__)
#define INFO(format, ...) log_info(format, ##__VA_ARGS__)
#define WARN(format, ...) log_warn(format, ##__VA_ARGS__)
#define WARNING(format, ...) log_warn(format, ##__VA_ARGS__) // Alias for WARN
#define ERROR(format, ...) log_error(format, ##__VA_ARGS__)

// Function declarations
void log_debug(const char *format, ...);
void log_info(const char *format, ...);
void log_warn(const char *format, ...);
void log_error(const char *format, ...);

// Initialization and control functions
void init_logging(log_level_t level);
void set_log_level(log_level_t level);
void set_log_file(const char *filepath);
void close_logging(void);

#endif /* LOG_UTILS_H */
