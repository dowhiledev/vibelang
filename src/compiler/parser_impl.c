/**
 * @file parser_impl.c
 * @brief Implementation of parser interface functions for the packrat parser
 */

#include <stdlib.h>
#include <string.h>
#include "../utils/ast.h"
#include "../utils/log_utils.h"
#include "parser.h"
#include "parser_internal.h"

/**
 * Simple helper to return the text from a packrat parser context.
 * This is used by the parser for handling string extractions.
 */
const char* text(const void* auxil) {
    // In the packrat parser context, auxil is typically a pointer to the string
    if (auxil == NULL) {
        return NULL;
    }
    return (const char*)auxil;
}

/**
 * Create a new Vibe parser context
 *
 * @param auxil Auxiliary data (typically the source string to parse)
 * @return A new parser context
 */
vibe_context_t *vibe_create(void* auxil) {
    DEBUG("Creating vibe context with auxil=%p", auxil);
    
    // Allocate memory for the context
    vibe_context_t *ctx = (vibe_context_t*)malloc(sizeof(vibe_context_t));
    if (ctx == NULL) {
        ERROR("Failed to allocate memory for parser context");
        return NULL;
    }
    
    // Initialize context fields
    memset(ctx, 0, sizeof(vibe_context_t));
    ctx->auxil = auxil;  // Store the source string
    ctx->pos = 0;        // Start at position 0
    ctx->buffer_capacity = 4096;  // Default buffer capacity
    ctx->buffer = (char*)malloc(ctx->buffer_capacity);
    
    if (ctx->buffer == NULL) {
        ERROR("Failed to allocate memory for parser buffer");
        free(ctx);
        return NULL;
    }
    
    DEBUG("Vibe context created successfully: %p", (void*)ctx);
    return ctx;
}

/**
 * Parse input using the Vibe parser
 *
 * @param ctx The parser context
 * @param ret Pointer to store the resulting AST
 * @return 0 on success, non-zero on failure
 */
int vibe_parse(vibe_context_t *ctx, ast_node_t **ret) {
    if (ctx == NULL || ret == NULL) {
        ERROR("Invalid parameters to vibe_parse");
        return -1;
    }
    
    // In a real implementation, this would call the actual parsing logic
    // For now, we'll just return a simple success/failure
    DEBUG("Parsing with context %p", (void*)ctx);
    
    // For testing purposes, we'll create a minimal program AST
    ast_node_t* program = create_ast_node(AST_PROGRAM);
    if (!program) {
        ERROR("Failed to create program node");
        return -1;
    }
    
    *ret = program;
    return 0;
}

/**
 * Destroy a Vibe parser context
 *
 * @param ctx The parser context to destroy
 */
void vibe_destroy(vibe_context_t *ctx) {
    if (ctx == NULL) {
        return;
    }
    
    DEBUG("Destroying vibe context: %p", (void*)ctx);
    
    // Free any allocated memory in the context
    if (ctx->buffer) {
        free(ctx->buffer);
        ctx->buffer = NULL;
    }
    
    // Free the context itself
    free(ctx);
}
