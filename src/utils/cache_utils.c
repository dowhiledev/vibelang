#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "cache_utils.h"
#include "file_utils.h"
#include "log_utils.h"

#define DEFAULT_CACHE_DIR ".vibelang_cache"

static char* cache_directory = NULL;

/* Initialize the cache system */
void cache_init(const char* cache_dir) {
    if (cache_directory) {
        free(cache_directory);
    }
    
    if (cache_dir) {
        cache_directory = strdup(cache_dir);
    } else {
        // Use default cache directory in user's home
        const char* home = getenv("HOME");
        if (!home) {
            home = ".";  // Fallback to current directory
        }
        
        cache_directory = path_join(home, DEFAULT_CACHE_DIR);
    }
    
    // Create cache directory if it doesn't exist
    if (!file_exists(cache_directory)) {
        DEBUG("Creating cache directory: %s", cache_directory);
        if (!create_directories(cache_directory)) {
            ERROR("Failed to create cache directory: %s", cache_directory);
        }
    }
}

/* Clean up cache resources */
void cache_cleanup(void) {
    if (cache_directory) {
        free(cache_directory);
        cache_directory = NULL;
    }
}

/* Get cache directory */
const char* cache_get_dir(void) {
    if (!cache_directory) {
        cache_init(NULL);  // Initialize with default
    }
    return cache_directory;
}

/* Check if cache needs update */
int cache_needs_update(const char* input_file, const char* output_file) {
    if (!file_exists(output_file)) {
        return 1;  // Output file doesn't exist, needs update
    }
    
    // Compare modification times
    long long input_time = get_file_mtime(input_file);
    long long output_time = get_file_mtime(output_file);
    
    if (input_time < 0 || output_time < 0) {
        return 1;  // Error getting file times, assume update needed
    }
    
    // Check if input is newer than output
    return input_time > output_time;
}

/* Get cached file path */
char* cache_get_path(const char* module_name, const char* extension) {
    if (!cache_directory) {
        cache_init(NULL);  // Initialize with default
    }
    
    // Create filename: module_name.extension
    char* filename;
    if (extension && extension[0] != '.') {
        filename = malloc(strlen(module_name) + strlen(extension) + 2);
        sprintf(filename, "%s.%s", module_name, extension);
    } else if (extension) {
        filename = malloc(strlen(module_name) + strlen(extension) + 1);
        sprintf(filename, "%s%s", module_name, extension);
    } else {
        filename = strdup(module_name);
    }
    
    // Join with cache directory
    char* result = path_join(cache_directory, filename);
    free(filename);
    
    return result;
}

/* Clear cache for a specific module */
void cache_clear_module(const char* module_name) {
    // TODO: Implement removing cached files for a specific module
    // This would require tracking all generated files for a module
    ERROR("cache_clear_module not yet implemented");
}

/* Clear all cache entries */
void cache_clear_all(void) {
    if (!cache_directory) {
        cache_init(NULL);
    }
    
    // TODO: Implement recursive directory removal
    ERROR("cache_clear_all not yet implemented");
}
