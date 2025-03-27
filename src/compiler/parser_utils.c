#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/ast.h"
#include "../utils/log_utils.h"
#include "parser.h"

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
    
    // No need to cast, since we updated the header to accept const char*
    vibe_context_t* ctx = vibe_create(source);
    if (!ctx) {
        ERROR("Failed to create parser context");
        return NULL;
    }
    
    ast_node_t* ast = NULL;
    int result = vibe_parse(ctx, &ast);
    
    if (!result || !ast) {
        ERROR("Parsing failed: %s", vibe_get_error(ctx));
        vibe_destroy(ctx);
        return NULL;
    }
    
    vibe_destroy(ctx);
    return ast;
}
