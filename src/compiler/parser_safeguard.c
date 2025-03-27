#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/log_utils.h"
#include "parser.h"
#include "parser_utils.h"

// Maximum number of ast nodes in a single parse
#define MAX_AST_NODES 10000

// Global counter to detect runaway parser
static int ast_node_counter = 0;

// Wrap ast_node_create to detect potential infinite loops
ast_node_t* safe_ast_node_create(ast_node_type_t type) {
    // Check limit to prevent runaway parser
    if (ast_node_counter > MAX_AST_NODES) {
        ERROR("Parser exceeded maximum AST node limit, likely an infinite loop");
        return NULL;
    }
    
    ast_node_counter++;
    return create_ast_node(type);
}

// Reset the counter between parse operations
void reset_ast_node_counter() {
    ast_node_counter = 0;
}

// Safer version of the parse function
ast_node_t* safe_parse_string(const char* source) {
    if (!source) return NULL;
    
    // Reset counters before parsing
    reset_ast_node_counter();
    
    // Get AST from normal parser
    ast_node_t* result = parse_string(source);
    
    // Log diagnostics
    DEBUG("Parsed with %d AST nodes created", ast_node_counter);
    
    return result;
}
