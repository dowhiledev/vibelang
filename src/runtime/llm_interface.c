/**
 * @file llm_interface.c
 * @brief Implementation of LLM API communications
 */

#include "llm_interface.h"
#include "../utils/log_utils.h"
#include "config.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Memory struct for cURL response data
typedef struct {
  char *memory;
  size_t size;
} ResponseMemory;

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

  // Check if API key is set
  const char *api_key = get_api_key();
  if (!api_key || strlen(api_key) == 0) {
    ERROR("API key not set");
    return 0;
  }

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
 * Send a prompt to the LLM and get the response
 *
 * @param prompt The formatted prompt to send
 * @param meaning The semantic meaning of the prompt (used for response
 * processing)
 * @return The response from the LLM, or NULL on error
 */
char *send_llm_prompt(const char *prompt, const char *meaning) {
  if (!prompt)
    return NULL;

  DEBUG("Sending prompt to LLM: %s", prompt);

  CURL *curl;
  CURLcode res;
  ResponseMemory chunk = {0};

  // Initialize the memory chunk
  chunk.memory = malloc(1);
  chunk.size = 0;

  // Initialize cURL session
  curl = curl_easy_init();
  if (!curl) {
    ERROR("Failed to initialize cURL");
    free(chunk.memory);
    return NULL;
  }

  // Get API key
  const char *api_key = get_api_key();
  if (!api_key || strlen(api_key) == 0) {
    ERROR("API key not set");
    curl_easy_cleanup(curl);
    free(chunk.memory);
    return NULL;
  }

  // For now, just mock the LLM response for testing
  // In a real implementation, this would make an actual API call
  if (strstr(prompt, "weather") || strstr(meaning, "weather")) {
    free(chunk.memory);
    return strdup("Sunny with a high of 75°F");
  } else if (strstr(prompt, "temperature") || strstr(meaning, "temperature")) {
    free(chunk.memory);
    return strdup("72°F");
  } else if (strstr(prompt, "greeting") || strstr(meaning, "greeting")) {
    free(chunk.memory);
    char *formatted = malloc(strlen(prompt) + 100);
    if (formatted) {
      strcpy(formatted, "Hello! ");
      strcat(formatted, prompt);
    }
    return formatted;
  }

  // Default mock response - fix the string concatenation
  free(chunk.memory);

  // Allocate memory for combined string
  char *response = malloc(strlen("LLM response for: ") + strlen(prompt) + 1);
  if (response) {
    sprintf(response, "LLM response for: %s", prompt);
    return response;
  }

  // In case of malloc failure, return a simple string
  return strdup("Error generating LLM response");
}

/**
 * Close the LLM connection
 */
void close_llm_connection(void) {
  DEBUG("Closing LLM connection");
  curl_global_cleanup();
}
