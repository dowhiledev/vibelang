#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "../include/vibelang.h"
#include "utils/log_utils.h"

/* Forward declarations from runtime module */
extern VibeError vibelang_init(void);
extern void vibelang_shutdown();
extern VibeModule* vibelang_load(const char* filename);
extern void vibelang_unload(VibeModule* module);
extern const char* vibe_get_error_message();

/* Initialize library */
__attribute__((constructor))
static void init() {
    set_log_level(LOG_LEVEL_INFO);
    INFO("VibeLang library loaded");
}

/* Cleanup on library unload */
__attribute__((destructor))
static void cleanup() {
    INFO("VibeLang library unloaded");
}

/**
 * Main library interface implementation for VibeLanguage
 *
 * This file implements the main public API of the VibeLanguage compiler
 * and runtime.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/vibelang.h"
#include "../include/runtime.h"
#include "../src/utils/ast.h"
#include "../src/utils/log_utils.h"
#include "../src/compiler/parser_utils.h"
#include "../src/compiler/semantic.h"
#include "../src/compiler/codegen.h"

// Expose key functions from the internal modules
// These are needed because macOS has different linking behavior than Linux
// and sometimes internal symbols from static libs aren't exported properly

// Function to wrap the runtime prompt execution
// This implementation needs to match the declaration in runtime.h
// We'll declare this as extern to avoid duplicate definitions
extern VibeValue vibe_execute_prompt(const char* prompt, const char* meaning);

// Expose parser function - already defined in parser_utils.c
extern ast_node_t* parse_string(const char* source);

// Expose semantic analyzer
int vibe_analyze_semantics(ast_node_t* ast) {
    return analyze_semantics(ast);
}

// Expose code generator
int vibe_generate_code(ast_node_t* ast, const char* output_file) {
    return generate_code(ast, output_file);
}

// Expose AST handling
void vibe_free_ast(ast_node_t* ast) {
    ast_node_free(ast);
}

// Compile source to C code
int vibelang_compile(const char* source, const char* output_file) {
    INFO("Compiling VibeLanguage to C...");
    
    // Parse source
    ast_node_t* ast = parse_string(source);
    if (!ast) {
        ERROR("Failed to parse input");
        return -1;
    }
    
    // Analyze semantics
    if (analyze_semantics(ast) != 0) {
        ERROR("Semantic analysis failed");
        ast_node_free(ast);
        return -1;
    }
    
    // Generate code
    if (!generate_code(ast, output_file)) {
        ERROR("Code generation failed");
        ast_node_free(ast);
        return -1;
    }
    
    ast_node_free(ast);
    return 0;
}

// Initialize the library
VibeError vibelang_init(void) {
    // Initialize logging
    init_logging(LOG_LEVEL_INFO);
    INFO("VibeLanguage library initialized");
    return VIBE_SUCCESS;
}

// Shutdown the library
void vibelang_shutdown(void) {
    INFO("VibeLanguage library shutdown");
    close_logging();
}

// Create a new module
VibeModule* vibe_create_module(const char* name, const char* source_path) {
    VibeModule* module = malloc(sizeof(VibeModule));
    if (!module) {
        ERROR("Failed to allocate memory for module");
        return NULL;
    }
    
    module->name = name ? strdup(name) : NULL;
    module->source_path = source_path ? strdup(source_path) : NULL;
    module->output_path = NULL;
    module->internal_data = NULL;
    
    return module;
}

// Free a module
void vibe_free_module(VibeModule* module) {
    if (module) {
        if (module->name) free(module->name);
        if (module->source_path) free(module->source_path);
        if (module->output_path) free(module->output_path);
        if (module->internal_data) free(module->internal_data);
        free(module);
    }
}

// Value creation functions are defined in runtime.c
extern VibeValue vibe_string_value(const char* str);
extern VibeValue vibe_number_value(double num);
extern VibeValue vibe_bool_value(int b);
extern VibeValue vibe_null_value(void);
extern VibeValue vibe_int_value(int value);
extern VibeValue vibe_float_value(double value);
