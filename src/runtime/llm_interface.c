/**
 * @file llm_interface.c
 * @brief Implementation of LLM API communications
 */

#include "llm_interface.h"
#include "../utils/log_utils.h"
#include "config.h"
#include <cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Memory struct for cURL response data
typedef struct {
  char *memory;
  size_t size;
} ResponseMemory;

// Global cURL handle
static CURL *curl_handle = NULL;
static char error_buffer[CURL_ERROR_SIZE];

// cURL write callback function
static size_t write_memory_callback(void *contents, size_t size, size_t nmemb,
                                    void *userp) {
  size_t real_size = size * nmemb;
  ResponseMemory *mem = (ResponseMemory *)userp;

  char *ptr = realloc(mem->memory, mem->size + real_size + 1);
  if (!ptr) {
    ERROR("Not enough memory for cURL response");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, real_size);
  mem->size += real_size;
  mem->memory[mem->size] = 0;

  return real_size;
}

/**
 * Initialize the LLM connection
 */
int init_llm_connection(void) {
  DEBUG("Initializing LLM connection");

  // Initialize cURL
  CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (res != CURLE_OK) {
    ERROR("curl_global_init() failed: %s", curl_easy_strerror(res));
    return 0;
  }

  // Create a persistent cURL handle
  curl_handle = curl_easy_init();
  if (!curl_handle) {
    ERROR("curl_easy_init() failed");
    curl_global_cleanup();
    return 0;
  }

  // Check for API key in environment variables first (higher priority)
  const char *env_api_key = getenv("OPENAI_API_KEY");
  const char *generic_env_key = getenv("VIBELANG_API_KEY");

  if ((env_api_key && strlen(env_api_key) > 0) ||
      (generic_env_key && strlen(generic_env_key) > 0)) {
    DEBUG("Found API key in environment variables");
  } else {
    // Fall back to the config file
    const char *api_key = get_api_key();
    if (!api_key || strlen(api_key) == 0) {
      ERROR("API key not set. Please set it in vibeconfig.json or via "
            "OPENAI_API_KEY environment variable");
      curl_easy_cleanup(curl_handle);
      curl_handle = NULL;
      curl_global_cleanup();
      return 0;
    }
  }

  // Set up error buffer
  curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error_buffer);

  // Set a timeout of 30 seconds to prevent hanging on network issues
  curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30L);

  DEBUG("LLM connection initialized successfully");
  return 1;
}

/**
 * Format a prompt template with variable values
 */
char *format_prompt(const char *template, char **var_names, char **var_values,
                    int var_count) {
  if (!template)
    return NULL;
  if (var_count == 0 || !var_names || !var_values) {
    return strdup(template); // No variables to substitute
  }

  // Calculate the size of the formatted prompt
  size_t formatted_size =
      strlen(template) + 1; // Start with template size + null terminator
  size_t template_len = strlen(template);

  // First pass: calculate the required size
  for (int i = 0; i < var_count; i++) {
    char var_marker[256];
    snprintf(var_marker, sizeof(var_marker), "{%s}", var_names[i]);

    const char *pos = template;
    while ((pos = strstr(pos, var_marker)) != NULL) {
      // Remove marker size, add variable value size
      formatted_size =
          formatted_size - strlen(var_marker) + strlen(var_values[i]);
      pos += strlen(var_marker);
    }
  }

  // Allocate memory for the formatted prompt
  char *formatted = malloc(formatted_size);
  if (!formatted) {
    ERROR("Failed to allocate memory for formatted prompt");
    return NULL;
  }

  // Initialize with template
  strcpy(formatted, template);

  // Second pass: perform substitutions
  for (int i = 0; i < var_count; i++) {
    char var_marker[256];
    snprintf(var_marker, sizeof(var_marker), "{%s}", var_names[i]);

    char *pos;
    while ((pos = strstr(formatted, var_marker)) != NULL) {
      size_t marker_len = strlen(var_marker);
      size_t value_len = strlen(var_values[i]);

      // Shift the remaining string to accommodate the value
      if (marker_len != value_len) {
        memmove(pos + value_len, pos + marker_len,
                strlen(pos + marker_len) + 1);
      }

      // Copy the value in place of the marker
      memcpy(pos, var_values[i], value_len);
    }
  }

  return formatted;
}

/**
 * Parse the OpenAI API JSON response to extract the content message
 *
 * @param json_str The JSON response string from OpenAI API
 * @return The extracted content message, or NULL on error
 */
static char *parse_openai_response(const char *json_str) {
  if (!json_str) {
    ERROR("NULL JSON response");
    return NULL;
  }

  DEBUG("Parsing OpenAI JSON response");

  cJSON *root = cJSON_Parse(json_str);
  if (!root) {
    ERROR("Failed to parse JSON response: %s", cJSON_GetErrorPtr());
    return NULL;
  }

  // First, get the "choices" array
  cJSON *choices = cJSON_GetObjectItem(root, "choices");
  if (!choices || !cJSON_IsArray(choices) || cJSON_GetArraySize(choices) == 0) {
    ERROR("Invalid or empty choices array in response");
    cJSON_Delete(root);
    return NULL;
  }

  // Get the first choice (we only expect one in typical responses)
  cJSON *choice = cJSON_GetArrayItem(choices, 0);
  if (!choice) {
    ERROR("Failed to get first choice from response");
    cJSON_Delete(root);
    return NULL;
  }

  // Get the "message" object
  cJSON *message = cJSON_GetObjectItem(choice, "message");
  if (!message) {
    ERROR("Message object not found in choice");
    cJSON_Delete(root);
    return NULL;
  }

  // Get the "content" string
  cJSON *content = cJSON_GetObjectItem(message, "content");
  if (!content || !cJSON_IsString(content)) {
    ERROR("Content not found or not a string");
    cJSON_Delete(root);
    return NULL;
  }

  // Extract the content string
  char *result = strdup(content->valuestring);

  // Clean up JSON objects
  cJSON_Delete(root);

  DEBUG("Successfully extracted message content from JSON");
  return result;
}

/**
 * Send a prompt to the LLM and get the response
 *
 * @param prompt The formatted prompt to send
 * @param meaning The semantic meaning of the prompt (used for response
 * processing)
 * @return The response from the LLM, or NULL on error
 */
char *send_llm_prompt(const char *prompt, const char *meaning) {
  // Very early debug to see if we get this far
  fprintf(stderr, "DEBUG: Entering send_llm_prompt with prompt='%s'\n",
          prompt ? prompt : "NULL");

  if (!prompt) {
    fprintf(stderr, "ERROR: NULL prompt provided to send_llm_prompt\n");
    return NULL;
  }

  DEBUG("Sending prompt to LLM: %s", prompt);

  // Check for dev mode FIRST, before any API key checks
  const char *dev_mode = getenv("VIBELANG_DEV_MODE");
  fprintf(stderr, "DEBUG: VIBELANG_DEV_MODE=%s\n",
          dev_mode ? dev_mode : "NULL");

  if (dev_mode && strcmp(dev_mode, "1") == 0) {
    fprintf(stderr, "DEBUG: Using mock LLM responses (dev mode)\n");

    // Safe check for strings before using strstr
    int has_weather_term = 0;
    if (prompt) {
      has_weather_term = (strstr(prompt, "weather") != NULL);
    }

    if (meaning) {
      has_weather_term =
          has_weather_term || (strstr(meaning, "weather") != NULL);
    }

    // Mock responses for testing based on keywords
    if (has_weather_term) {
      fprintf(stderr, "DEBUG: Returning mock weather response\n");
      return strdup("Sunny with a high of 75°F");
    }

    int has_temp_term = 0;
    if (prompt) {
      has_temp_term = (strstr(prompt, "temperature") != NULL);
    }

    if (meaning) {
      has_temp_term = has_temp_term || (strstr(meaning, "temperature") != NULL);
    }

    if (has_temp_term) {
      DEBUG("Returning temperature mock response");
      return strdup("25");
    }

    int has_greeting_term = 0;
    if (prompt) {
      has_greeting_term = (strstr(prompt, "greeting") != NULL);
    }

    if (meaning) {
      has_greeting_term =
          has_greeting_term || (strstr(meaning, "greeting") != NULL);
    }

    if (has_greeting_term) {
      DEBUG("Returning greeting mock response");
      char *formatted = malloc(100 + (prompt ? strlen(prompt) : 0));
      if (formatted) {
        strcpy(formatted, "Hello! Welcome to VibeLang.");
        return formatted;
      }
    }

    // Default mock response - always returns something in dev mode
    fprintf(stderr, "DEBUG: Returning default mock response\n");
    return strdup("This is a mock response from the LLM");
  }

  // After this point we're in production mode, so check API keys
  const char *api_key = get_api_key();
  const char *env_api_key = getenv("OPENAI_API_KEY");

  // Use environment variable if available, otherwise use config
  if (env_api_key && strlen(env_api_key) > 0) {
    api_key = env_api_key;
    DEBUG("Using API key from environment variable");
  } else {
    DEBUG("Using API key from config file");
  }

  if (!api_key || strlen(api_key) == 0) {
    ERROR("API key not set");
    return NULL;
  }

  // Setup cURL for a real API call
  if (!curl_handle) {
    ERROR("curl_handle is NULL, LLM connection not initialized");
    return NULL;
  }

  // Reset the curl handle for a fresh request
  curl_easy_reset(curl_handle);

  // Set the URL for OpenAI's API
  curl_easy_setopt(curl_handle, CURLOPT_URL,
                   "https://api.openai.com/v1/chat/completions");

  // Set timeout and error buffer
  curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30L);
  curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, error_buffer);

  // Prepare headers
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  char auth_header[256];
  snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s",
           api_key);
  headers = curl_slist_append(headers, auth_header);

  curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

  // Prepare the request JSON
  char *json_payload = malloc(strlen(prompt) + 200);
  if (!json_payload) {
    ERROR("Failed to allocate memory for JSON payload");
    curl_slist_free_all(headers);
    return NULL;
  }

  // Create a simple JSON payload for the OpenAI Chat API
  snprintf(json_payload, strlen(prompt) + 200,
           "{\"model\":\"gpt-3.5-turbo\",\"messages\":[{\"role\":\"user\","
           "\"content\":\"%s\"}],\"temperature\":0.7}",
           prompt);

  curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, json_payload);

  // Response data structure
  ResponseMemory chunk;
  chunk.memory = malloc(1);
  chunk.size = 0;

  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  // Perform the request
  CURLcode res = curl_easy_perform(curl_handle);

  // Cleanup
  curl_slist_free_all(headers);
  free(json_payload);

  if (res != CURLE_OK) {
    ERROR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    if (chunk.memory)
      free(chunk.memory);
    return NULL;
  }

  long response_code;
  curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);

  if (response_code != 200) {
    ERROR("API request failed with HTTP code %ld: %s", response_code,
          chunk.memory ? chunk.memory : "unknown error");
    if (chunk.memory)
      free(chunk.memory);
    return NULL;
  }

  // For non-development mode, parse the OpenAI response
  if (!dev_mode || strcmp(dev_mode, "0") == 0) {
    DEBUG("Parsing OpenAI API response (production mode)");
    char *parsed_content = parse_openai_response(chunk.memory);
    free(chunk.memory); // Free the raw response

    if (!parsed_content) {
      ERROR("Failed to parse API response");
      return NULL;
    }

    return parsed_content;
  }

  // For development mode, return the raw response (which is already a plain
  // text mock)
  return chunk.memory;
}

/**
 * Close the LLM connection
 */
void close_llm_connection(void) {
  DEBUG("Closing LLM connection");

  if (curl_handle) {
    curl_easy_cleanup(curl_handle);
    curl_handle = NULL;
  }

  curl_global_cleanup();
}
