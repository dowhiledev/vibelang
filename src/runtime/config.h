/**
 * @file config.h
 * @brief Configuration management for the Vibe language runtime
 */

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Load configuration from the default configuration file
 * 
 * @return 1 on success, 0 on failure
 */
int load_config(void);

/**
 * Get the API key from the loaded configuration
 * 
 * @return The API key string, or NULL if not found or not loaded
 */
const char* get_api_key(void);

/**
 * Free all resources allocated for the configuration
 */
void free_config(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */
