#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/ast.h"
#include "../utils/log_utils.h"
#include "parser.h"

// Function to get error message from parser context
const char* vibe_get_error(vibe_context_t* ctx) {
    return "Parser error"; // Simple error message since we can't modify parser.h
}

/**
 * Parse a string and return the AST.
 * This is a utility function for tests and REPL.
 *
 * @param source The source code to parse
 * @return The AST root node, or NULL on error
 */
ast_node_t* parse_string(const char* source) {
    if (!source) {
        ERROR("NULL source provided to parse_string");
        return NULL;
    }
    
    // Cast const char* to void* to match the function signature
    // The parser treats it internally as a string
    vibe_context_t* ctx = vibe_create((void*)source);
    if (!ctx) {
        ERROR("Failed to create parser context");
        return NULL;
    }
    
    // Using an int pointer to receive the packed AST node pointer
    int result_value = 0;
    int parse_success = vibe_parse(ctx, &result_value);
    
    // The parser stores pointers as integers, so we need to convert it back
    ast_node_t* ast = NULL;
    if (parse_success && result_value != 0) {
        // Convert the integer value back to a pointer
        ast = (ast_node_t*)(intptr_t)result_value;
    }
    
    if (!parse_success || !ast) {
        ERROR("Parsing failed");
        vibe_destroy(ctx);
        return NULL;
    }
    
    vibe_destroy(ctx);
    return ast;
}
