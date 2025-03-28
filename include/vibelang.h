/**
 * @file vibelang.h
 * @brief Main public API for the VibeLanguage compiler and runtime
 */

#ifndef VIBELANG_H
#define VIBELANG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ast.h"  // For ast_node_t

/**
 * Error codes returned by VibeLanguage API functions
 */
typedef enum VibeError {
    VIBE_SUCCESS = 0,
    VIBE_ERROR_GENERAL = -1,
    VIBE_ERROR_PARSER = -2,
    VIBE_ERROR_SEMANTIC = -3,
    VIBE_ERROR_CODEGEN = -4,
    VIBE_ERROR_RUNTIME = -5,
    VIBE_ERROR_IO = -6,
    VIBE_ERROR_LLM_CONNECTION_FAILED = -7  // Added this error code
} VibeError;

/**
 * Represents a compiled Vibe module
 */
typedef struct VibeModule {
    char* name;           // Module name
    char* source_path;    // Source file path
    char* output_path;    // Output file path
    void* internal_data;  // Internal compiler data
} VibeModule;

/**
 * Represents a runtime value in Vibe
 */
typedef struct VibeValue {
    enum {
        VIBE_STRING,
        VIBE_NUMBER,
        VIBE_BOOLEAN,
        VIBE_NULL,
        VIBE_OBJECT
    } type;
    
    union {
        char* string_val;
        double number_val;
        int bool_val;
        void* object_val;
    } data;
} VibeValue;

/**
 * Initialize the VibeLanguage library
 *
 * @return VIBE_SUCCESS on success, error code on failure
 */
VibeError vibelang_init(void);

/**
 * Shutdown the VibeLanguage library
 */
void vibelang_shutdown(void);

/**
 * Compile VibeLanguage source code to C
 *
 * @param source The VibeLanguage source code
 * @param output_file The output C file path
 * @return 0 on success, non-zero on error
 */
int vibelang_compile(const char* source, const char* output_file);

/**
 * Parse VibeLanguage source code into an AST
 * 
 * @param source The VibeLanguage source code
 * @return The root AST node, or NULL on error
 */
ast_node_t* vibe_parse_string(const char* source);

/**
 * Perform semantic analysis on an AST
 * 
 * @param ast The root AST node
 * @return 0 on success, non-zero on error
 */
int vibe_analyze_semantics(ast_node_t* ast);

/**
 * Generate C code from an AST
 * 
 * @param ast The root AST node
 * @param output_file The output C file path
 * @return 1 on success, 0 on error
 */
int vibe_generate_code(ast_node_t* ast, const char* output_file);

/**
 * Free an AST
 * 
 * @param ast The root AST node
 */
void vibe_free_ast(ast_node_t* ast);

/**
 * Create a new VibeModule from source
 *
 * @param name Module name
 * @param source_path Path to source file
 * @return Pointer to new module, NULL on failure
 */
VibeModule* vibe_create_module(const char* name, const char* source_path);

/**
 * Free a VibeModule
 * 
 * @param module The module to free
 */
void vibe_free_module(VibeModule* module);

/**
 * Create a new VibeValue with string type
 *
 * @param str The string value
 * @return A new VibeValue
 */
VibeValue vibe_string_value(const char* str);

/**
 * Create a new VibeValue with numeric type
 *
 * @param num The numeric value
 * @return A new VibeValue
 */
VibeValue vibe_number_value(double num);

/**
 * Create a new VibeValue with boolean type
 *
 * @param b The boolean value
 * @return A new VibeValue
 */
VibeValue vibe_bool_value(int b);

/**
 * Create a new VibeValue with null type
 *
 * @return A new VibeValue of null type
 */
VibeValue vibe_null_value(void);

/**
 * Create a new VibeValue with integer type
 * (Convenience function - uses number type internally)
 *
 * @param value The integer value
 * @return A new VibeValue
 */
VibeValue vibe_int_value(int value);

/**
 * Create a new VibeValue with float type
 * (Convenience function - uses number type internally)
 *
 * @param value The float value
 * @return A new VibeValue
 */
VibeValue vibe_float_value(double value);

#ifdef __cplusplus
}
#endif

#endif /* VIBELANG_H */
