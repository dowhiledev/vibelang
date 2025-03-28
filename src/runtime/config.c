/**
 * @file config.c
 * @brief Configuration management for the Vibe language runtime
 */

#include "config.h"
#include "../utils/log_utils.h"
#include "../vendor/cjson/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Default configuration file path
#define CONFIG_FILE_PATH "vibeconfig.json"

// Global variables to store configuration
static char *api_key = NULL;
static char *model_name = NULL;
static int max_tokens = 2048;

// Forward declaration of create_default_config function
static int create_default_config(void);

/**
 * Load configuration from the default configuration file
 *
 * @return 1 on success, 0 on failure
 */
int load_config(void) {
  INFO("Loading configuration from %s", CONFIG_FILE_PATH);

  // Read the configuration file
  FILE *config_file = fopen(CONFIG_FILE_PATH, "r");
  if (!config_file) {
    WARN("Configuration file not found: %s", CONFIG_FILE_PATH);
    // Create a default config if none exists
    return create_default_config();
  }

  // Get file size
  fseek(config_file, 0, SEEK_END);
  long file_size = ftell(config_file);
  rewind(config_file);

  // Read file content
  char *config_content = (char *)malloc(file_size + 1);
  if (!config_content) {
    ERROR("Memory allocation failed for config content");
    fclose(config_file);
    return 0;
  }

  size_t read_size = fread(config_content, 1, file_size, config_file);
  fclose(config_file);

  if (read_size != file_size) {
    ERROR("Failed to read configuration file");
    free(config_content);
    return 0;
  }

  config_content[file_size] = '\0';

  // Parse JSON
  cJSON *json = cJSON_Parse(config_content);
  free(config_content);

  if (!json) {
    ERROR("Failed to parse configuration file: %s", cJSON_GetErrorPtr());
    return 0;
  }

  // Extract values
  cJSON *api_key_json = cJSON_GetObjectItem(json, "api_key");
  if (cJSON_IsString(api_key_json) && api_key_json->valuestring != NULL) {
    api_key = strdup(api_key_json->valuestring);
  }

  cJSON *model_name_json = cJSON_GetObjectItem(json, "model_name");
  if (cJSON_IsString(model_name_json) && model_name_json->valuestring != NULL) {
    model_name = strdup(model_name_json->valuestring);
  }

  cJSON *max_tokens_json = cJSON_GetObjectItem(json, "max_tokens");
  if (cJSON_IsNumber(max_tokens_json)) {
    max_tokens = max_tokens_json->valueint;
  }

  cJSON_Delete(json);

  // Validate configuration
  if (!api_key || strlen(api_key) == 0) {
    WARN("API key is missing in configuration");
    return 0;
  }

  INFO("Configuration loaded successfully");
  return 1;
}

/**
 * Create a default configuration file
 *
 * @return 1 on success, 0 on failure
 */
static int create_default_config(void) {
  INFO("Creating default configuration file: %s", CONFIG_FILE_PATH);

  // Set default values
  api_key = strdup("YOUR_API_KEY_HERE"); // Placeholder
  model_name = strdup("gpt-4");
  max_tokens = 2048;

  // Create JSON
  cJSON *json = cJSON_CreateObject();
  cJSON_AddStringToObject(json, "api_key", api_key);
  cJSON_AddStringToObject(json, "model_name", model_name);
  cJSON_AddNumberToObject(json, "max_tokens", max_tokens);

  // Write to file
  char *config_content = cJSON_Print(json);
  cJSON_Delete(json);

  if (!config_content) {
    ERROR("Failed to generate JSON configuration");
    return 0;
  }

  FILE *config_file = fopen(CONFIG_FILE_PATH, "w");
  if (!config_file) {
    ERROR("Failed to create configuration file: %s", CONFIG_FILE_PATH);
    free(config_content);
    return 0;
  }

  fputs(config_content, config_file);
  fclose(config_file);
  free(config_content);

  INFO("Default configuration file created. Please edit %s and set your API "
       "key.",
       CONFIG_FILE_PATH);
  return 1;
}

/**
 * Get the API key from the loaded configuration
 *
 * @return The API key string, or NULL if not found or not loaded
 */
const char *get_api_key(void) { return api_key; }

/**
 * Free all resources allocated for the configuration
 */
void free_config(void) {
  if (api_key) {
    free(api_key);
    api_key = NULL;
  }
  if (model_name) {
    free(model_name);
    model_name = NULL;
  }
  INFO("Configuration resources freed");
}
