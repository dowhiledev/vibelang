#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "../utils/log_utils.h"

/* Forward declarations */
extern const char* get_llm_provider();
extern const char* get_llm_api_key();
extern cJSON* get_llm_params_for_function(const char* function_name);
extern void free_llm_params(cJSON* params);

/* CURL response struct */
typedef struct {
    char* data;
    size_t size;
} response_t;

/* CURL callback for receiving data */
static size_t curl_write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t real_size = size * nmemb;
    response_t* resp = (response_t*)userdata;
    
    // Reallocate memory for the response data
    char* new_data = realloc(resp->data, resp->size + real_size + 1);
    if (!new_data) {
        ERROR("Memory allocation failed in CURL callback");
        return 0;  // Signal error to libcurl
    }
    
    resp->data = new_data;
    memcpy(&resp->data[resp->size], ptr, real_size);
    resp->size += real_size;
    resp->data[resp->size] = '\0';  // Null-terminate
    
    return real_size;
}

/* Initialize LLM interface */
int init_llm_interface() {
    curl_global_init(CURL_GLOBAL_ALL);
    return 1;
}

/* Clean up LLM interface */
void cleanup_llm_interface() {
    curl_global_cleanup();
}

/* Send prompt to OpenAI */
static char* send_openai_prompt(const char* prompt, cJSON* params) {
    const char* api_key = get_llm_api_key();
    if (!api_key) {
        ERROR("OpenAI API key not found. Set OPENAI_API_KEY environment variable or in config.");
        return NULL;
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        ERROR("Failed to initialize CURL");
        return NULL;
    }
    
    // Prepare request
    const char* url = "https://api.openai.com/v1/chat/completions";
    
    // Extract model parameter or use default
    const char* model = "gpt-3.5-turbo";
    cJSON* model_json = cJSON_GetObjectItem(params, "model");
    if (model_json && cJSON_IsString(model_json)) {
        model = model_json->valuestring;
    }
    
    // Create request body
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "model", model);
    
    // Add other parameters from config
    cJSON* child = NULL;
    cJSON_ArrayForEach(child, params) {
        if (strcmp(child->string, "model") != 0) {  // Skip model, already handled
            if (cJSON_IsNumber(child)) {
                cJSON_AddNumberToObject(root, child->string, child->valuedouble);
            } else if (cJSON_IsString(child)) {
                cJSON_AddStringToObject(root, child->string, child->valuestring);
            } else if (cJSON_IsBool(child)) {
                cJSON_AddBoolToObject(root, child->string, cJSON_IsTrue(child));
            }
        }
    }
    
    // Add messages array with system and user messages
    cJSON* messages = cJSON_AddArrayToObject(root, "messages");
    
    // System message
    cJSON* system_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(system_msg, "role", "system");
    cJSON_AddStringToObject(system_msg, "content", "You are a helpful assistant.");
    cJSON_AddItemToArray(messages, system_msg);
    
    // User message with the prompt
    cJSON* user_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(user_msg, "role", "user");
    cJSON_AddStringToObject(user_msg, "content", prompt);
    cJSON_AddItemToArray(messages, user_msg);
    
    char* json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    
    if (!json_str) {
        ERROR("Failed to generate JSON request");
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    // Set up CURL
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
    headers = curl_slist_append(headers, auth_header);
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Set up response handling
    response_t response = { .data = malloc(1), .size = 0 };
    if (!response.data) {
        ERROR("Memory allocation failed for response");
        free(json_str);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return NULL;
    }
    response.data[0] = '\0';
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    // Clean up request resources
    free(json_str);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        ERROR("CURL request failed: %s", curl_easy_strerror(res));
        free(response.data);
        return NULL;
    }
    
    DEBUG("Received response from OpenAI: %s", response.data);
    
    // Parse response to extract the completion text
    cJSON* resp_json = cJSON_Parse(response.data);
    free(response.data);
    
    if (!resp_json) {
        ERROR("Failed to parse JSON response");
        return NULL;
    }
    
    // Extract the completion text from choices[0].message.content
    char* completion = NULL;
    cJSON* choices = cJSON_GetObjectItem(resp_json, "choices");
    if (choices && cJSON_IsArray(choices) && cJSON_GetArraySize(choices) > 0) {
        cJSON* first_choice = cJSON_GetArrayItem(choices, 0);
        if (first_choice) {
            cJSON* message = cJSON_GetObjectItem(first_choice, "message");
            if (message) {
                cJSON* content = cJSON_GetObjectItem(message, "content");
                if (content && cJSON_IsString(content)) {
                    completion = strdup(content->valuestring);
                }
            }
        }
    }
    
    cJSON_Delete(resp_json);
    
    if (!completion) {
        ERROR("Failed to extract completion from response");
    }
    
    return completion;
}

/* Execute a prompt with the current LLM provider */
char* execute_prompt(const char* prompt, const char* function_name) {
    if (!prompt) {
        ERROR("Null prompt provided to execute_prompt");
        return NULL;
    }
    
    // Get LLM configuration
    const char* provider = get_llm_provider();
    cJSON* params = get_llm_params_for_function(function_name);
    
    char* result = NULL;
    
    // Dispatch to the appropriate provider
    if (strcmp(provider, "OpenAI") == 0) {
        result = send_openai_prompt(prompt, params);
    } else {
        ERROR("Unsupported LLM provider: %s", provider);
    }
    
    free_llm_params(params);
    return result;
}

/* Replace placeholders in a prompt template */
char* format_prompt(const char* template, char** var_names, char** var_values, int var_count) {
    if (!template) return NULL;
    
    // Compute required size
    size_t result_size = strlen(template) + 1;
    for (int i = 0; i < var_count; i++) {
        if (!var_names[i] || !var_values[i]) continue;
        
        // Find all occurrences of {var_name}
        char placeholder[256];
        snprintf(placeholder, sizeof(placeholder), "{%s}", var_names[i]);
        
        const char* pos = template;
        while ((pos = strstr(pos, placeholder)) != NULL) {
            // Add difference between value length and placeholder length
            result_size += strlen(var_values[i]) - strlen(placeholder);
            pos += strlen(placeholder);
        }
    }
    
    // Allocate result string
    char* result = malloc(result_size);
    if (!result) {
        ERROR("Memory allocation failed in format_prompt");
        return NULL;
    }
    strcpy(result, template);
    
    // Replace each variable
    for (int i = 0; i < var_count; i++) {
        if (!var_names[i] || !var_values[i]) continue;
        
        char placeholder[256];
        snprintf(placeholder, sizeof(placeholder), "{%s}", var_names[i]);
        
        char* pos;
        while ((pos = strstr(result, placeholder)) != NULL) {
            // Shift rest of string to accommodate value
            size_t tail_len = strlen(pos + strlen(placeholder));
            memmove(pos + strlen(var_values[i]), pos + strlen(placeholder), tail_len + 1);
            memcpy(pos, var_values[i], strlen(var_values[i]));
        }
    }
    
    return result;
}
