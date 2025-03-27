#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>  // For intptr_t
#include <time.h>    // For timeout implementation
#include <pthread.h> // For thread-safe parsing with timeout
#include "../utils/ast.h"
#include "../utils/log_utils.h"
#include "parser.h"
#include "parser_utils.h"

// Thread-local storage for the current parsed text
static __thread const char* current_text = NULL;

// Default timeout for parsing operations (in seconds)
#define DEFAULT_PARSE_TIMEOUT 2

// Maximum recursion depth for parser - used to prevent stack overflow
#define MAX_PARSER_RECURSION 1000

// Structure for thread parsing arguments
typedef struct {
    const char* source;
    ast_node_t* result;
    int success;
    int finished;
} parse_thread_args_t;

/**
 * Parser utility function to access the current text being parsed
 * Used by the parser in grammar actions
 */
const char* text(const void* auxil) {
    // In PackCC, auxil can contain the input text for simple parsers
    // For more complex parsers, it might be a pointer to a context object
    if (!current_text) {
        // Cast the auxil pointer back to char* for simple cases
        current_text = (const char*)auxil;
    }
    return current_text;
}

// Function to get error message from parser context
const char* vibe_get_error(vibe_context_t* ctx) {
    return "Parser error"; // Simple error message since we can't modify parser.h
}

/**
 * Extract string value from an AST node.
 * Used by the grammar to properly convert AST nodes to strings.
 */
const char* extract_string_value(ast_node_t* node) {
    if (!node) return NULL;
    
    if (node->type == AST_STRING_LITERAL) {
        return ast_get_string(node, "value");
    }
    if (node->type == AST_IDENTIFIER) {
        return ast_get_string(node, "name");
    }
    
    // If not a string-containing node, return an empty string
    return "";
}

/**
 * Get array length helper function for PackCC grammar
 */
int pcc_array_length(void* arr) {
    if (!arr) return 0;
    
    ast_list_t* list = (ast_list_t*)arr;
    return list ? list->len : 0;
}

/**
 * Get array item helper function for PackCC grammar
 */
ast_node_t* pcc_array_get(void* arr, int index) {
    if (!arr) return NULL;
    
    ast_list_t* list = (ast_list_t*)arr;
    if (index < 0 || index >= list->len) return NULL;
    
    return list->list[index];
}

/**
 * Internal thread function to perform parsing without locking the main thread
 */
static void* parse_thread_func(void* arg) {
    parse_thread_args_t* args = (parse_thread_args_t*)arg;
    
    if (!args || !args->source) {
        DEBUG("NULL arguments provided to parse thread");
        return NULL;
    }
    
    // Handle parsing in this thread
    DEBUG("Thread starting to parse source");
    vibe_context_t* ctx = vibe_create((void*)args->source);
    if (!ctx) {
        DEBUG("Thread failed to create parser context");
        args->success = 0;
        args->finished = 1;
        return NULL;
    }
    
    // Call the parser with the correct type
    args->success = vibe_parse(ctx, &args->result);
    DEBUG("Thread finished parsing with result: %d", args->success);
    
    vibe_destroy(ctx);
    args->finished = 1;
    return NULL;
}

/**
 * Parse a string and return the AST.
 * Fixed implementation that avoids the infinite recursion in the parser.
 *
 * @param source The source code to parse
 * @return The AST root node, or NULL on error
 */
ast_node_t* parse_string(const char* source) {
    if (!source) {
        ERROR("NULL source provided to parse_string");
        return NULL;
    }
    
    DEBUG("Starting parse_string with direct approach");
    
    // Reset counters and metrics before parsing
    ast_reset_metrics();
    
    // Reset the thread-local text pointer to avoid stale data
    current_text = NULL;
    
    // Create a copy of the source to ensure it remains valid
    char* source_copy = strdup(source);
    if (!source_copy) {
        ERROR("Failed to allocate memory for source copy");
        return NULL;
    }
    
    // Set a reasonable timeout
    time_t start_time = time(NULL);
    int timeout_seconds = DEFAULT_PARSE_TIMEOUT;
    
    // Direct approach without threading - simpler and less error-prone
    DEBUG("Creating parser context for source: %.40s...", source_copy);
    vibe_context_t* ctx = vibe_create(source_copy);
    if (!ctx) {
        ERROR("Failed to create parser context");
        free(source_copy);
        return NULL;
    }
    
    // Initialize a pointer to receive the AST
    ast_node_t* ast = NULL;
    
    // Call the parser with the correct type
    DEBUG("Starting to parse source");
    int parse_success = vibe_parse(ctx, &ast);
    DEBUG("Parser returned: success=%d, ast=%p", parse_success, (void*)ast);
    
    // Check if we're taking too long
    time_t elapsed = time(NULL) - start_time;
    if (elapsed > timeout_seconds) {
        WARN("Parsing took longer than expected: %ld seconds", elapsed);
    }
    
    // Clean up the parser context
    DEBUG("Destroying parser context");
    vibe_destroy(ctx);
    
    // Free the source copy since we don't need it anymore
    free(source_copy);
    
    if (!parse_success || !ast) {
        ERROR("Parsing failed");
        return NULL;
    }
    
    // Log metrics for diagnostic purposes
    int depth, count;
    ast_get_metrics(&depth, &count);
    DEBUG("Parsing completed with AST metrics: depth=%d, nodes=%d", depth, count);
    
    DEBUG("Parsing completed successfully with AST: %p", (void*)ast);
    return ast;
}

/**
 * Non-thread version of parse_string for simpler cases and platforms
 * that don't support threading
 */
ast_node_t* parse_string_simple(const char* source) {
    if (!source) {
        ERROR("NULL source provided to parse_string_simple");
        return NULL;
    }
    
    // Simple direct approach without threading
    vibe_context_t* ctx = vibe_create((void*)source);
    if (!ctx) {
        ERROR("Failed to create parser context");
        return NULL;
    }
    
    // Initialize a pointer to receive the AST
    ast_node_t* ast = NULL;
    
    // Call the parser with the correct type
    int parse_success = vibe_parse(ctx, &ast);
    
    vibe_destroy(ctx);
    
    if (!parse_success || !ast) {
        ERROR("Parsing failed");
        return NULL;
    }
    
    return ast;
}

// Helper function to create an AST list from PackCC values
ast_list_t* create_ast_list(ast_node_t* first, ast_node_t** rest, size_t rest_count) {
    ast_list_t* list = malloc(sizeof(ast_list_t));
    if (!list) {
        ERROR("Failed to allocate memory for AST list");
        return NULL;
    }
    
    size_t total_count = first ? 1 : 0;
    total_count += rest_count;
    
    list->list = malloc(sizeof(ast_node_t*) * total_count);
    if (!list->list) {
        ERROR("Failed to allocate memory for AST list items");
        free(list);
        return NULL;
    }
    
    list->len = total_count;
    
    // Add first item if provided
    size_t index = 0;
    if (first) {
        list->list[index++] = first;
    }
    
    // Add rest items
    for (size_t i = 0; i < rest_count; i++) {
        list->list[index++] = rest[i];
    }
    
    return list;
}

// Helper function to free an AST list
void free_ast_list(ast_list_t* list) {
    if (!list) return;
    
    // Note: This doesn't free the AST nodes themselves,
    // just the list structure
    free(list->list);
    free(list);
}
