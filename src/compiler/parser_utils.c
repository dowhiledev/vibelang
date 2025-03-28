#include "parser_utils.h"
#include "../utils/ast.h"
#include "../utils/log_utils.h"
#include "parser.tab.h" // Include the generated Bison header
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// External declarations from flex/bison
extern int yyparse();
extern void yylex_reset();
extern YY_BUFFER_STATE
yy_scan_string(const char *); // Fix: use the correct Flex type
extern void yy_delete_buffer(
    YY_BUFFER_STATE); // Fix: specify the correct parameter type
extern ast_node_t *parse_result;

// Only declare parse_string here, don't define it - it's already defined in
// parser.y
extern ast_node_t *parse_string(const char *source);

// Memory pool for AST nodes during parsing
static ast_node_t **node_pool = NULL;
static size_t node_pool_size = 0;
static size_t node_pool_capacity = 0;

// Initialize the memory pool
void init_parser_memory() {
  if (node_pool == NULL) {
    node_pool_capacity = 1024; // Initial capacity
    node_pool = malloc(sizeof(ast_node_t *) * node_pool_capacity);
    node_pool_size = 0;
  }
}

// Track an AST node for cleanup
void track_ast_node(ast_node_t *node) {
  if (node_pool_size >= node_pool_capacity) {
    node_pool_capacity *= 2;
    node_pool = realloc(node_pool, sizeof(ast_node_t *) * node_pool_capacity);
  }
  node_pool[node_pool_size++] = node;
}

// Free all tracked AST nodes
void cleanup_parser_memory() {
  for (size_t i = 0; i < node_pool_size; i++) {
    ast_node_free(node_pool[i]);
  }
  free(node_pool);
  node_pool = NULL;
  node_pool_size = 0;
  node_pool_capacity = 0;
}

/**
 * Extract string value from an AST node.
 * Used by the grammar to properly convert AST nodes to strings.
 */
const char *extract_string_value(ast_node_t *node) {
  if (!node)
    return NULL;

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
 * Helper function to create an AST list
 */
ast_list_t *create_ast_list(ast_node_t *first, ast_node_t **rest,
                            size_t rest_count) {
  ast_list_t *list = malloc(sizeof(ast_list_t));
  if (!list) {
    ERROR("Failed to allocate memory for AST list");
    return NULL;
  }

  size_t total_count = first ? 1 : 0;
  total_count += rest_count;

  list->list = malloc(sizeof(ast_node_t *) * total_count);
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

/**
 * Helper function to free an AST list
 */
void free_ast_list(ast_list_t *list) {
  if (!list)
    return;

  // Note: This doesn't free the AST nodes themselves,
  // just the list structure
  free(list->list);
  free(list);
}
