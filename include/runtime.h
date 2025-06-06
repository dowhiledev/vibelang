/**
 * @file runtime.h
 * @brief Runtime functions for the Vibe language
 */

#ifndef RUNTIME_H
#define RUNTIME_H

#include "vibelang.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the Vibe language runtime. This is called automatically the first
 * time a generated function executes, but may be invoked explicitly to check
 * for errors or override configuration.
 *
 * @return VIBE_SUCCESS on success, error code on failure
 */
VibeError vibe_runtime_init(void);

/**
 * Shutdown the Vibe language runtime. It will also be called automatically at
 * program exit.
 */
void vibe_runtime_shutdown(void);

/**
 * Execute a prompt with a specific meaning context
 *
 * @param prompt The prompt template to execute
 * @param meaning The semantic meaning context
 * @return A VibeValue containing the result
 */
VibeValue vibe_execute_prompt(const char *prompt, const char *meaning);

/**
 * Load a compiled module
 *
 * @param module_name Name of the module to load
 * @return Pointer to the loaded module, NULL on failure
 */
VibeModule *vibe_load_module(const char *module_name);

/**
 * Unload a module
 *
 * @param module The module to unload
 */
void vibe_unload_module(VibeModule *module);

/**
 * Call a function within a module
 *
 * @param module The module containing the function
 * @param function_name Name of the function to call
 * @param args Array of arguments
 * @param arg_count Number of arguments
 * @return The function's return value
 */
VibeValue *vibe_call_function(VibeModule *module, const char *function_name,
                              VibeValue *args, int arg_count);

/**
 * Create a null value
 *
 * @return A VibeValue with null type
 */
VibeValue vibe_null_value(void);

/**
 * Get string value from a VibeValue
 *
 * @param value The value to extract from
 * @return The string value or empty string if not a string
 */
const char *vibe_get_string(VibeValue *value);

/**
 * Get number value from a VibeValue
 *
 * @param value The value to extract from
 * @return The number value or 0 if not a number
 */
double vibe_get_number(VibeValue *value);

/**
 * Get integer value from a VibeValue
 *
 * @param value The value to extract from
 * @return The integer value or 0 if not convertible
 */
int vibe_value_get_int(VibeValue *value);

/**
 * Get boolean value from a VibeValue
 *
 * @param value The value to extract from
 * @return The boolean value or 0 if not a boolean
 */
int vibe_get_bool(VibeValue *value);

#ifdef __cplusplus
}
#endif

#endif /* RUNTIME_H */
