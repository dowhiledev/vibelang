#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>  // For intptr_t
#include "../utils/ast.h"
#include "../utils/log_utils.h"
#include "parser.h"
#include "parser_utils.h"

// Thread-local storage for the current parsed text
// This is used by the text() function to access the current text being parsed
static __thread const char* current_text = NULL;

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
    
    // No need to cast const char* to void* anymore since we've updated the signature
    vibe_context_t* ctx = vibe_create(source);
    if (!ctx) {
        ERROR("Failed to create parser context");
        return NULL;
    }
    
    // Initialize a pointer to receive the AST
    ast_node_t* ast = NULL;
    
    // Call the parser with the correct type
    int parse_success = vibe_parse(ctx, &ast);
    
    if (!parse_success || !ast) {
        ERROR("Parsing failed");
        vibe_destroy(ctx);
        return NULL;
    }
    
    vibe_destroy(ctx);
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
