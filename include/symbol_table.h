#ifndef VIBELANG_SYMBOL_TABLE_H
#define VIBELANG_SYMBOL_TABLE_H

#include "../src/utils/ast.h"
#include <stddef.h>

typedef enum {
  SYM_TYPE,      // Type definition
  SYM_FUNCTION,  // Function declaration
  SYM_VAR,       // Variable declaration
  SYM_PARAMETER, // Function parameter
  SYM_CLASS      // Class declaration
} symbol_kind_t;

typedef struct symbol {
  char *name;
  symbol_kind_t kind;
  ast_node_t *node;           // AST node for this symbol
  ast_node_t *type_node;      // Type AST node (if applicable)
  struct symbol_scope *scope; // Containing scope
  struct symbol *next;        // Next symbol in the same scope (linked list)
} symbol_t;

typedef struct symbol_scope {
  symbol_t *symbols;           // Linked list of symbols in this scope
  struct symbol_scope *parent; // Parent scope
  ast_node_t *node; // AST node for this scope (e.g., function, block)
} symbol_scope_t;

/* Symbol table functions */
symbol_scope_t *create_symbol_scope(symbol_scope_t *parent, ast_node_t *node);
void free_symbol_scope(symbol_scope_t *scope);

symbol_t *symbol_add(symbol_scope_t *scope, const char *name,
                     symbol_kind_t kind, ast_node_t *node,
                     ast_node_t *type_node);
symbol_t *symbol_lookup(symbol_scope_t *scope, const char *name);
symbol_t *symbol_lookup_local(symbol_scope_t *scope, const char *name);

void print_symbol_table(symbol_scope_t *scope, int depth);

#endif /* VIBELANG_SYMBOL_TABLE_H */
