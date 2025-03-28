/**
 * @file config.c
 * @brief Configuration management for the Vibe language runtime
 */

#include "config.h"
#include "../utils/file_utils.h"
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
static int config_loaded = 0;

// Forward declaration of create_default_config function
static int create_default_config(void);

/**
 * Load configuration from the default configuration file
 *
 * @return 1 on success, 0 on failure
 */
int load_config(void) {
  INFO("Loading configuration from %s", CONFIG_FILE_PATH);

  // Initialize with default values so we don't crash if config is missing
  if (!api_key)
    api_key = strdup("YOUR_API_KEY_HERE"); // Default placeholder
  if (!model_name)
    model_name = strdup("gpt-3.5-turbo"); // Default model

  // Check environment variables first - they override config file
  const char *env_api_key = getenv("OPENAI_API_KEY");
  const char *generic_env_key = getenv("VIBELANG_API_KEY");

  if (env_api_key && strlen(env_api_key) > 0) {
    DEBUG("Using API key from OPENAI_API_KEY environment variable");
    if (api_key)
      free(api_key);
    api_key = strdup(env_api_key);
    config_loaded = 1;
    return 1;
  } else if (generic_env_key && strlen(generic_env_key) > 0) {
    DEBUG("Using API key from VIBELANG_API_KEY environment variable");
    if (api_key)
      free(api_key);
    api_key = strdup(generic_env_key);
    config_loaded = 1;
    return 1;
  }

  // Read the configuration file
  if (!file_exists(CONFIG_FILE_PATH)) {
    WARN("Configuration file not found: %s", CONFIG_FILE_PATH);
    // We already initialized with default values, so we can continue
    config_loaded = 1;
    return 1; // Return success to avoid crashing, we'll use defaults
  }

  FILE *config_file = fopen(CONFIG_FILE_PATH, "r");
  if (!config_file) {
    WARN("Could not open configuration file: %s", CONFIG_FILE_PATH);
    // We already initialized with default values, so we can continue
    config_loaded = 1;
    return 1;
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

  // Extract API key - first check global section
  cJSON *global = cJSON_GetObjectItem(json, "global");
  if (global) {
    cJSON *api_key_json = cJSON_GetObjectItem(global, "api_key");
    if (cJSON_IsString(api_key_json) && api_key_json->valuestring != NULL) {
      // Free previous value if it exists
      if (api_key)
        free(api_key);
      api_key = strdup(api_key_json->valuestring);
    }

    // Get model name from default_params if available
    cJSON *default_params = cJSON_GetObjectItem(global, "default_params");
    if (default_params) {
      cJSON *model_name_json = cJSON_GetObjectItem(default_params, "model");
      if (cJSON_IsString(model_name_json) &&
          model_name_json->valuestring != NULL) {
        // Free previous value if it exists
        if (model_name)
          free(model_name);
        model_name = strdup(model_name_json->valuestring);
      }

      cJSON *max_tokens_json =
          cJSON_GetObjectItem(default_params, "max_tokens");
      if (cJSON_IsNumber(max_tokens_json)) {
        max_tokens = max_tokens_json->valueint;
      }
    }
  }

  cJSON_Delete(json);
  config_loaded = 1;

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
  if (api_key)
    free(api_key);
  api_key = strdup("YOUR_API_KEY_HERE"); // Placeholder

  if (model_name)
    free(model_name);
  model_name = strdup("gpt-3.5-turbo");

  max_tokens = 2048;

  // Create JSON
  cJSON *json = cJSON_CreateObject();
  cJSON *global = cJSON_CreateObject();
  cJSON_AddItemToObject(json, "global", global);

  cJSON_AddStringToObject(global, "api_key", api_key);

  cJSON *default_params = cJSON_CreateObject();
  cJSON_AddItemToObject(global, "default_params", default_params);

  cJSON_AddStringToObject(default_params, "model", model_name);
  cJSON_AddNumberToObject(default_params, "max_tokens", max_tokens);
  cJSON_AddNumberToObject(default_params, "temperature", 0.7);

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
const char *get_api_key(void) {
  // Ensure config is loaded
  if (!config_loaded) {
    load_config();
  }
  return api_key;
}

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
  config_loaded = 0;
  INFO("Configuration resources freed");
}
