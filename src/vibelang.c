#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "../include/vibelang.h"
#include "utils/log_utils.h"

/* Forward declarations from runtime module */
extern VibeError vibelang_init();
extern void vibelang_shutdown();
extern VibeModule* vibelang_load(const char* filename);
extern void vibelang_unload(VibeModule* module);
// Remove or comment out this line: extern VibeValue* vibe_call_raw(VibeModule* module, const char* function_name, ...);
extern const char* vibe_get_error_message();

/* Initialize library */
__attribute__((constructor))
static void init() {
    log_init(LOG_LEVEL_INFO);
    INFO("VibeLang library loaded");
}

/* Cleanup on library unload */
__attribute__((destructor))
static void cleanup() {
    INFO("VibeLang library unloaded");
}

/* Proxy functions for the public API - use the version from runtime.c */
/*
VibeValue* vibe_call_with_args(VibeModule* module, const char* function_name, 
                             VibeValue** args, size_t arg_count) {
    if (!module || !function_name) {
        ERROR("Invalid arguments to vibe_call_with_args");
        return NULL;
    }
    
    INFO("Calling function %s with %zu arguments", function_name, arg_count);
    
    // TODO: Implement proper function calling with arguments
    // This is a placeholder that will be completed in the next phase
    return vibe_value_null();
}
*/

/* Variadic wrapper for vibe_call_with_args - remove or comment out to avoid duplication */
/*
VibeValue* vibe_call(VibeModule* module, const char* function_name, ...) {
    // TODO: Implement variadic argument handling
    // For now, just call without arguments
    return vibe_call_with_args(module, function_name, NULL, 0);
}
*/

/* 
 * Note: All other API functions (vibelang_init, vibelang_shutdown, etc.)
 * are implemented in runtime.c and exported directly.
 */

/* Execute a prompt and return the result as a VibeValue */
VibeValue* vibe_execute_prompt(const char* prompt) {
    if (!prompt) {
        ERROR("Null prompt provided to vibe_execute_prompt");
        return NULL;
    }
    
    // Forward to the underlying implementation
    extern char* execute_prompt(const char* prompt, const char* function_name);
    char* result = execute_prompt(prompt, NULL);
    
    if (!result) {
        return vibe_value_null();
    }
    
    VibeValue* value = vibe_value_string(result);
    free(result);
    return value;
}
