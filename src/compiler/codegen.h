/**
 * @file codegen.h
 * @brief Code generation functions for the Vibe language compiler
 *
 * This file contains the definitions for functions that generate C code
 * from the semantically validated AST.
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "../utils/ast.h"

/**
 * Generate code from the AST and write it to an output file
 *
 * @param ast The root AST node
 * @param output_file The path to the output file
 * @return 1 on success, 0 on error
 */
int generate_code(ast_node_t *ast, const char *output_file);

/**
 * Generate code for a function declaration
 *
 * @param func The function declaration AST node
 * @param file The file to write to
 * @return 1 on success, 0 on error
 */
int generate_function(ast_node_t *func, FILE *file);

/**
 * Generate code for a type declaration
 *
 * @param type_decl The type declaration AST node
 * @param file The file to write to
 * @return 1 on success, 0 on error
 */
int generate_type_declaration(ast_node_t *type_decl, FILE *file);

/**
 * Generate code for the prompt block
 *
 * @param prompt The prompt block AST node
 * @param file The file to write to
 * @return 1 on success, 0 on error
 */
int generate_prompt_block(ast_node_t *prompt, FILE *file);

/**
 * Generate code for an expression
 *
 * @param expr The expression AST node
 * @param file The file to write to
 * @return 1 on success, 0 on error
 */
int generate_expression(ast_node_t *expr, FILE *file);

/**
 * Generate code for a statement
 *
 * @param stmt The statement AST node
 * @param file The file to write to
 * @param indent The indentation level
 * @return 1 on success, 0 on error
 */
int generate_statement(ast_node_t *stmt, FILE *file, int indent);

/**
 * Generate the required runtime headers and includes
 *
 * @param file The file to write to
 * @return 1 on success, 0 on error
 */
int generate_headers(FILE *file);

#endif /* CODEGEN_H */
