#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include "../utils/ast.h"

// Forward declaration of the parser context type
typedef struct vibe_context_tag vibe_context_t;

// Function to access text directly from the parser - used by grammar actions
const char* text(const void* auxil);

// Helper function to extract string values from AST nodes
const char* extract_string_value(ast_node_t* node);

// Helper functions for handling arrays in PackCC
int pcc_array_length(void* arr);
ast_node_t* pcc_array_get(void* arr, int index);

// Function to parse a string into an AST
ast_node_t* parse_string(const char* source);

// Helper function to get the error message from a parser context
const char* vibe_get_error(vibe_context_t* ctx);

// Helper functions for AST list manipulation
ast_list_t* create_ast_list(ast_node_t* first, ast_node_t** rest, size_t rest_count);
void free_ast_list(ast_list_t* list);

// Definitions for text access in the generated parser
// These macros will be used in the grammar.peg file
#define pcc_yytext text(auxil)
#define pcc_yylen (strlen(text(auxil)) - 2) // Length minus quotes

#endif /* PARSER_UTILS_H */
