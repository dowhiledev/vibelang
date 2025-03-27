#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "../utils/log_utils.h"
#include "../utils/file_utils.h"

/* LLM Configuration structure */
typedef struct {
    char* provider;
    char* api_key;
    cJSON* default_params;
    cJSON* function_overrides;
} llm_config_t;

/* Global config */
static llm_config_t global_config = {0};

/* Initialize configuration with defaults */
static void init_default_config() {
    if (global_config.provider) {
        free(global_config.provider);
    }
    global_config.provider = strdup("OpenAI");
    
    if (global_config.api_key) {
        free(global_config.api_key);
    }
    global_config.api_key = NULL;  // Will be read from environment
    
    if (global_config.default_params) {
        cJSON_Delete(global_config.default_params);
    }
    
    global_config.default_params = cJSON_CreateObject();
    cJSON_AddStringToObject(global_config.default_params, "model", "gpt-3.5-turbo");
    cJSON_AddNumberToObject(global_config.default_params, "temperature", 0.7);
    cJSON_AddNumberToObject(global_config.default_params, "max_tokens", 150);
    
    if (global_config.function_overrides) {
        cJSON_Delete(global_config.function_overrides);
    }
    global_config.function_overrides = cJSON_CreateObject();
}

/* Load configuration from file */
int load_config_from_file(const char* filename) {
    // Initialize with defaults first
    init_default_config();
    
    // Check if config file exists
    if (!file_exists(filename)) {
        WARNING("Config file '%s' not found, using defaults", filename);
        return 0;
    }
    
    // Read JSON file
    char* json_text = read_file(filename);
    if (!json_text) {
        ERROR("Failed to read config file '%s'", filename);
        return 0;
    }
    
    // Parse JSON
    cJSON* root = cJSON_Parse(json_text);
    free(json_text);
    
    if (!root) {
        ERROR("Failed to parse config file '%s': %s", 
             filename, cJSON_GetErrorPtr() ? cJSON_GetErrorPtr() : "unknown error");
        return 0;
    }
    
    // Extract global settings
    cJSON* global = cJSON_GetObjectItem(root, "global");
    if (global) {
        cJSON* provider = cJSON_GetObjectItem(global, "provider");
        if (provider && cJSON_IsString(provider)) {
            if (global_config.provider) {
                free(global_config.provider);
            }
            global_config.provider = strdup(provider->valuestring);
        }
        
        cJSON* api_key = cJSON_GetObjectItem(global, "api_key");
        if (api_key && cJSON_IsString(api_key)) {
            if (global_config.api_key) {
                free(global_config.api_key);
            }
            global_config.api_key = strdup(api_key->valuestring);
        }
        
        cJSON* params = cJSON_GetObjectItem(global, "default_params");
        if (params && cJSON_IsObject(params)) {
            cJSON_Delete(global_config.default_params);
            global_config.default_params = cJSON_Duplicate(params, 1);
        }
    }
    
    // Extract function overrides
    cJSON* overrides = cJSON_GetObjectItem(root, "overrides");
    if (overrides && cJSON_IsObject(overrides)) {
        cJSON_Delete(global_config.function_overrides);
        global_config.function_overrides = cJSON_Duplicate(overrides, 1);
    }
    
    cJSON_Delete(root);
    INFO("Loaded LLM config from '%s'", filename);
    
    return 1;
}

/* Clean up configuration */
void cleanup_config() {
    if (global_config.provider) {
        free(global_config.provider);
        global_config.provider = NULL;
    }
    
    if (global_config.api_key) {
        free(global_config.api_key);
        global_config.api_key = NULL;
    }
    
    if (global_config.default_params) {
        cJSON_Delete(global_config.default_params);
        global_config.default_params = NULL;
    }
    
    if (global_config.function_overrides) {
        cJSON_Delete(global_config.function_overrides);
        global_config.function_overrides = NULL;
    }
}

/* Get LLM provider name */
const char* get_llm_provider() {
    return global_config.provider;
}

/* Get API key */
const char* get_llm_api_key() {
    // Try config file first
    if (global_config.api_key) {
        return global_config.api_key;
    }
    
    // Fall back to environment variable
    const char* env_key = getenv("VIBELANG_API_KEY");
    if (env_key) {
        return env_key;
    }
    
    // Try provider-specific environment variables
    if (global_config.provider) {
        if (strcmp(global_config.provider, "OpenAI") == 0) {
            return getenv("OPENAI_API_KEY");
        } else if (strcmp(global_config.provider, "Anthropic") == 0) {
            return getenv("ANTHROPIC_API_KEY");
        }
    }
    
    return NULL;
}

/* Get parameter JSON for a function */
cJSON* get_llm_params_for_function(const char* function_name) {
    // Check for function-specific override
    if (function_name && global_config.function_overrides) {
        cJSON* override = cJSON_GetObjectItem(global_config.function_overrides, function_name);
        if (override) {
            // Merge with defaults
            cJSON* merged = cJSON_Duplicate(global_config.default_params, 1);
            
            cJSON* child = NULL;
            cJSON_ArrayForEach(child, override) {
                cJSON* existing = cJSON_GetObjectItem(merged, child->string);
                if (existing) {
                    cJSON_DeleteItemFromObject(merged, child->string);
                }
                cJSON_AddItemToObject(merged, child->string, cJSON_Duplicate(child, 1));
            }
            
            return merged;
        }
    }
    
    // Return copy of defaults
    return cJSON_Duplicate(global_config.default_params, 1);
}

/* Free parameter JSON */
void free_llm_params(cJSON* params) {
    if (params) {
        cJSON_Delete(params);
    }
}
