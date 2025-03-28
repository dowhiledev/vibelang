/**
 * @file semantic.h
 * @brief Semantic analysis functions for the Vibe language compiler
 * 
 * This file contains the definitions for functions that perform semantic
 * analysis on the AST produced by the parser.
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../utils/ast.h"
#include "../include/symbol_table.h"

/**
 * Perform semantic analysis on the AST
 * 
 * @param ast The root AST node to analyze
 * @return 0 on success, non-zero on error
 */
int analyze_semantics(ast_node_t* ast);

/**
 * Wrapper function for analyze_semantics to maintain API compatibility with tests
 * 
 * @param ast The root AST node to analyze
 * @return 0 on success, non-zero on error
 */
int semantic_analyze(ast_node_t* ast);

/**
 * Cleanup resources from semantic analysis
 */
void semantic_cleanup(void);

/**
 * Check if a node has the required type
 * 
 * @param node The AST node to check
 * @param expected_type The expected type string
 * @return 1 if the node has the expected type, 0 otherwise
 */
int check_node_type(ast_node_t* node, const char* expected_type);

/**
 * Validate function declarations in the AST
 * 
 * @param ast The AST to validate
 * @return 0 on success, non-zero on error
 */
int validate_functions(ast_node_t* ast);

/**
 * Validate type declarations in the AST
 * 
 * @param ast The AST to validate
 * @return 0 on success, non-zero on error
 */
int validate_types(ast_node_t* ast);

/**
 * Validate statements within a function body
 * 
 * @param body The function body AST node
 * @param symbol_table The current symbol table
 * @return 0 on success, non-zero on error
 */
int validate_statements(ast_node_t* body, symbol_scope_t* symbol_table);

/**
 * Validate an expression's type
 * 
 * @param expr The expression AST node
 * @param symbol_table The current symbol table
 * @param expected_type Expected type (can be NULL if no specific type is expected)
 * @return The resolved type string, or NULL on error
 */
char* validate_expression_type(ast_node_t* expr, symbol_scope_t* symbol_table, const char* expected_type);

#endif /* SEMANTIC_H */
