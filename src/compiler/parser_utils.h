#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include "../utils/ast.h"

// Include Flex buffer type definition
typedef struct yy_buffer_state* YY_BUFFER_STATE;

// Helper function to extract string values from AST nodes
const char* extract_string_value(ast_node_t* node);

// Function to parse a string into an AST using Bison/Flex
ast_node_t* parse_string(const char* source);

// Helper functions for AST list manipulation
ast_list_t* create_ast_list(ast_node_t* first, ast_node_t** rest, size_t rest_count);
void free_ast_list(ast_list_t* list);

// Memory management for parser
void init_parser_memory();
void track_ast_node(ast_node_t* node);
void cleanup_parser_memory();

#endif /* PARSER_UTILS_H */
