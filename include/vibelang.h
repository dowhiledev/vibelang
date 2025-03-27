/**
 * VibeLang Public C API
 * 
 * This header defines the public interface for integrating with VibeLang
 * from C applications.
 *
 * @file vibelang.h
 * @author VibeLang Team
 * @version 0.1.0
 */

#ifndef VIBELANG_H
#define VIBELANG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing a loaded VibeLang module
 */
typedef struct VibeModule VibeModule;

/**
 * @brief Structure representing a value in VibeLang
 */
typedef struct VibeValue VibeValue;

/**
 * @brief Error codes returned by VibeLang API functions
 */
typedef enum {
    VIBE_SUCCESS = 0,                 /**< Operation successful */
    VIBE_ERROR_FILE_NOT_FOUND,        /**< File not found */
    VIBE_ERROR_COMPILATION_FAILED,    /**< Compilation failed */
    VIBE_ERROR_FUNCTION_NOT_FOUND,    /**< Function not found in module */
    VIBE_ERROR_TYPE_MISMATCH,         /**< Type mismatch in function call */
    VIBE_ERROR_LLM_CONNECTION_FAILED, /**< Failed to connect to LLM provider */
    VIBE_ERROR_MEMORY_ALLOCATION      /**< Memory allocation failed */
} VibeError;

/**
 * @brief Value types supported by VibeLang
 */
typedef enum {
    VIBE_TYPE_NULL,    /**< Null value */
    VIBE_TYPE_BOOL,    /**< Boolean value */
    VIBE_TYPE_INT,     /**< Integer value */
    VIBE_TYPE_FLOAT,   /**< Floating point value */
    VIBE_TYPE_STRING,  /**< String value */
    VIBE_TYPE_ARRAY,   /**< Array of values */
    VIBE_TYPE_OBJECT   /**< Object with key-value pairs */
} VibeValueType;

/**
 * @brief Initialize the VibeLang runtime
 * 
 * This must be called before any other VibeLang functions.
 * 
 * @return VIBE_SUCCESS on success, error code on failure
 */
VibeError vibelang_init(void);

/**
 * @brief Shut down the VibeLang runtime
 * 
 * Should be called when the application is done using VibeLang
 * to free resources and clean up.
 */
void vibelang_shutdown(void);

/**
 * @brief Load a VibeLang module from file
 * 
 * The file can be either a .vibe source file or a compiled .so/.dll file.
 * If a source file is provided, it will be compiled automatically.
 * 
 * @param filename Path to the file to load
 * @return Pointer to the loaded module, or NULL on error
 */
VibeModule* vibelang_load(const char* filename);

/**
 * @brief Unload a VibeLang module
 * 
 * Frees resources associated with the module.
 * 
 * @param module The module to unload
 */
void vibelang_unload(VibeModule* module);

/**
 * @brief Call a function in a VibeLang module with variable arguments
 * 
 * @param module The module containing the function
 * @param function_name Name of the function to call
 * @param ... Variable arguments to pass to the function
 * @return Result value from the function, or NULL on error
 */
VibeValue* vibe_call(VibeModule* module, const char* function_name, ...);

/**
 * @brief Call a function in a VibeLang module with an array of arguments
 * 
 * @param module The module containing the function
 * @param function_name Name of the function to call
 * @param args Array of argument values
 * @param arg_count Number of arguments
 * @return Result value from the function, or NULL on error
 */
VibeValue* vibe_call_with_args(VibeModule* module, const char* function_name, VibeValue** args, size_t arg_count);

/**
 * @brief Create a null value
 * 
 * @return A new null value
 */
VibeValue* vibe_value_null(void);

/**
 * @brief Create a boolean value
 * 
 * @param value Boolean value (0 for false, non-zero for true)
 * @return A new boolean value
 */
VibeValue* vibe_value_bool(int value);

/**
 * @brief Create an integer value
 * 
 * @param value Integer value
 * @return A new integer value
 */
VibeValue* vibe_value_int(long long value);

/**
 * @brief Create a floating point value
 * 
 * @param value Floating point value
 * @return A new floating point value
 */
VibeValue* vibe_value_float(double value);

/**
 * @brief Create a string value
 * 
 * @param value String value (will be copied)
 * @return A new string value
 */
VibeValue* vibe_value_string(const char* value);

/**
 * @brief Get the type of a value
 * 
 * @param value The value to check
 * @return The type of the value
 */
VibeValueType vibe_value_get_type(const VibeValue* value);

/**
 * @brief Get a boolean value
 * 
 * @param value The value to get
 * @return The boolean value, or 0 if not a boolean
 */
int vibe_value_get_bool(const VibeValue* value);

/**
 * @brief Get an integer value
 * 
 * Will convert from other numeric types if possible.
 * 
 * @param value The value to get
 * @return The integer value, or 0 if not convertible
 */
long long vibe_value_get_int(const VibeValue* value);

/**
 * @brief Get a floating point value
 * 
 * Will convert from other numeric types if possible.
 * 
 * @param value The value to get
 * @return The floating point value, or 0.0 if not convertible
 */
double vibe_value_get_float(const VibeValue* value);

/**
 * @brief Get a string value
 * 
 * @param value The value to get
 * @return The string value, or NULL if not a string
 */
const char* vibe_value_get_string(const VibeValue* value);

/**
 * @brief Free a value
 * 
 * @param value The value to free
 */
void vibe_value_free(VibeValue* value);

/**
 * @brief Get the last error message
 * 
 * @return The last error message, or NULL if no error
 */
const char* vibe_get_error_message(void);

#ifdef __cplusplus
}
#endif

#endif /* VIBELANG_H */
