#ifndef VIBELANG_CACHE_UTILS_H
#define VIBELANG_CACHE_UTILS_H

#include <stddef.h>

/* Cache directory management */
void cache_init(const char *cache_dir);
void cache_cleanup(void);
const char *cache_get_dir(void);

/* Check if cache is up-to-date */
int cache_needs_update(const char *input_file, const char *output_file);

/* Get cached file path */
char *cache_get_path(const char *module_name, const char *extension);

/* Clear cache entries */
void cache_clear_module(const char *module_name);
void cache_clear_all(void);

#endif /* VIBELANG_CACHE_UTILS_H */
