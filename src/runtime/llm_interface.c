/**
 * @file llm_interface.c
 * @brief LLM (Language Model) interface functions for the Vibe language runtime
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../utils/log_utils.h"
#include "llm_interface.h"
#include "config.h"
#include "../vendor/cjson/cJSON.h"

// OpenAI API endpoint
#define OPENAI_API_ENDPOINT "https://api.openai.com/v1/chat/completions"

// Global variables
static int is_initialized = 0;

// Structure to store response data
typedef struct {
    char* data;
    size_t size;
} ResponseData;

// Forward declarations
static size_t write_callback(void* data, size_t size, size_t nmemb, void* userdata);
static char* send_openai_prompt(const char* prompt, const char* meaning);
static void cleanup_llm_interface(void);
static int init_llm_interface(void);

/**
 * Initialize the connection to the LLM service
 * 
 * @return 1 on success, 0 on failure
 */
int init_llm_connection(void) {
    if (is_initialized) {
        INFO("LLM interface already initialized");
        return 1;
    }
    
    INFO("Initializing LLM interface");
    return init_llm_interface();
}

/**
 * Close the connection to the LLM service
 */
void close_llm_connection(void) {
    if (!is_initialized) {
        return;
    }
    
    INFO("Closing LLM connection");
    cleanup_llm_interface();
    is_initialized = 0;
}

/**
 * Send a prompt to the LLM with a specific meaning context
 * 
 * @param prompt The text prompt to send
 * @param meaning The semantic meaning context
 * @return The response from the LLM, or NULL on error (caller must free)
 */
char* send_llm_prompt(const char* prompt, const char* meaning) {
    if (!is_initialized) {
        ERROR("LLM interface not initialized");
        return NULL;
    }
    
    if (!prompt || strlen(prompt) == 0) {
        ERROR("Empty prompt provided to send_llm_prompt");
        return NULL;
    }
    
    INFO("Sending prompt to LLM: %s (meaning: %s)", prompt, meaning ? meaning : "none");
    
    // Send the prompt to the actual service
    return send_openai_prompt(prompt, meaning);
}

/**
 * Initialize the LLM interface
 * 
 * @return 1 on success, 0 on failure
 */
static int init_llm_interface(void) {
    // Initialize libcurl
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        ERROR("Failed to initialize libcurl: %s", curl_easy_strerror(res));
        return 0;
    }
    
    is_initialized = 1;
    INFO("LLM interface initialized successfully");
    return 1;
}

/**
 * Clean up the LLM interface
 */
static void cleanup_llm_interface(void) {
    curl_global_cleanup();
    INFO("LLM interface resources freed");
}

/**
 * libcurl write callback function
 */
static size_t write_callback(void* data, size_t size, size_t nmemb, void* userdata) {
    ResponseData* response = (ResponseData*)userdata;
    size_t realsize = size * nmemb;
    
    char* ptr = realloc(response->data, response->size + realsize + 1);
    if (!ptr) {
        ERROR("Failed to allocate memory for response data");
        return 0;
    }
    
    response->data = ptr;
    memcpy(&(response->data[response->size]), data, realsize);
    response->size += realsize;
    response->data[response->size] = 0;
    
    return realsize;
}

/**
 * Send a prompt to OpenAI API
 * 
 * @param prompt The prompt to send
 * @param meaning The meaning context
 * @return The response text, or NULL on error
 */
static char* send_openai_prompt(const char* prompt, const char* meaning) {
    CURL* curl;
    CURLcode res;
    struct curl_slist* headers = NULL;
    char* response_text = NULL;
    ResponseData response_data = {0};
    char auth_header[256] = {0};
    
    // Create the API key header
    const char* api_key = get_api_key();
    if (!api_key || strlen(api_key) == 0) {
        ERROR("Missing API key");
        return NULL;
    }
    
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
    
    // Create the JSON request body
    cJSON* root = cJSON_CreateObject();
    cJSON* messages = cJSON_CreateArray();
    
    // Add system message for context if meaning is provided
    if (meaning && strlen(meaning) > 0) {
        cJSON* system_message = cJSON_CreateObject();
        cJSON_AddStringToObject(system_message, "role", "system");
        
        char system_content[512];
        snprintf(system_content, sizeof(system_content), 
                "You are providing information with the specific meaning of: %s. Respond with just the facts, no explanations.", 
                meaning);
        
        cJSON_AddStringToObject(system_message, "content", system_content);
        cJSON_AddItemToArray(messages, system_message);
    }
    
    // Add user message with the prompt
    cJSON* user_message = cJSON_CreateObject();
    cJSON_AddStringToObject(user_message, "role", "user");
    cJSON_AddStringToObject(user_message, "content", prompt);
    cJSON_AddItemToArray(messages, user_message);
    
    // Add messages to the request
    cJSON_AddItemToObject(root, "messages", messages);
    cJSON_AddStringToObject(root, "model", "gpt-4"); // Default model
    cJSON_AddNumberToObject(root, "max_tokens", 150);
    cJSON_AddNumberToObject(root, "temperature", 0.7);
    
    char* json_str = cJSON_Print(root);
    cJSON_Delete(root);
    
    if (!json_str) {
        ERROR("Failed to create JSON request");
        return NULL;
    }
    
    DEBUG("OpenAI API Request: %s", json_str);
    
    // Initialize curl session
    curl = curl_easy_init();
    if (!curl) {
        ERROR("Failed to initialize curl session");
        free(json_str);
        return NULL;
    }
    
    // Set up headers
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header);
    
    // Set curl options
    curl_easy_setopt(curl, CURLOPT_URL, OPENAI_API_ENDPOINT);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    // Perform the request
    res = curl_easy_perform(curl);
    
    // Clean up
    free(json_str);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        ERROR("curl_easy_perform failed: %s", curl_easy_strerror(res));
        if (response_data.data) free(response_data.data);
        return NULL;
    }
    
    // Parse the JSON response
    if (response_data.data) {
        DEBUG("OpenAI API Response: %s", response_data.data);
        
        cJSON* json = cJSON_Parse(response_data.data);
        if (json) {
            cJSON* choices = cJSON_GetObjectItem(json, "choices");
            if (cJSON_IsArray(choices) && cJSON_GetArraySize(choices) > 0) {
                cJSON* choice = cJSON_GetArrayItem(choices, 0);
                cJSON* message = cJSON_GetObjectItem(choice, "message");
                cJSON* content = cJSON_GetObjectItem(message, "content");
                
                if (cJSON_IsString(content) && content->valuestring != NULL) {
                    response_text = strdup(content->valuestring);
                    INFO("Received response from LLM: %s", response_text);
                }
            }
            cJSON_Delete(json);
        }
        
        free(response_data.data);
    }
    
    if (!response_text) {
        ERROR("Failed to parse response from LLM");
        return NULL;
    }
    
    return response_text;
}
