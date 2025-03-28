#include "../../include/symbol_table.h"
#include "../utils/log_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Create a new symbol scope */
symbol_scope_t *create_symbol_scope(symbol_scope_t *parent, ast_node_t *node) {
  symbol_scope_t *scope = (symbol_scope_t *)malloc(sizeof(symbol_scope_t));
  if (!scope) {
    ERROR("Memory allocation failed for symbol scope");
    return NULL;
  }

  scope->symbols = NULL;
  scope->parent = parent;
  scope->node = node;

  return scope;
}

/* Free a symbol scope and all its symbols */
void free_symbol_scope(symbol_scope_t *scope) {
  if (!scope)
    return;

  // Free symbols in this scope
  symbol_t *symbol = scope->symbols;
  while (symbol) {
    symbol_t *next = symbol->next;
    free(symbol->name);
    free(symbol);
    symbol = next;
  }

  free(scope);
}

/* Add a symbol to a scope */
symbol_t *symbol_add(symbol_scope_t *scope, const char *name,
                     symbol_kind_t kind, ast_node_t *node,
                     ast_node_t *type_node) {
  if (!scope || !name)
    return NULL;

  // Check for duplicates in this scope
  symbol_t *existing = symbol_lookup_local(scope, name);
  if (existing) {
    WARNING("Symbol '%s' is already defined in the current scope", name);
    return NULL;
  }

  // Create new symbol
  symbol_t *symbol = (symbol_t *)malloc(sizeof(symbol_t));
  if (!symbol) {
    ERROR("Memory allocation failed for symbol");
    return NULL;
  }

  symbol->name = strdup(name);
  symbol->kind = kind;
  symbol->node = node;
  symbol->type_node = type_node;
  symbol->scope = scope;

  // Add to the beginning of the linked list
  symbol->next = scope->symbols;
  scope->symbols = symbol;

  return symbol;
}

/* Look up a symbol in the current scope and all parent scopes */
symbol_t *symbol_lookup(symbol_scope_t *scope, const char *name) {
  if (!scope || !name)
    return NULL;

  // Check current scope
  symbol_t *symbol = scope->symbols;
  while (symbol) {
    if (strcmp(symbol->name, name) == 0) {
      return symbol;
    }
    symbol = symbol->next;
  }

  // Check parent scope
  if (scope->parent) {
    return symbol_lookup(scope->parent, name);
  }

  return NULL; // Symbol not found
}

/* Look up a symbol in the current scope only */
symbol_t *symbol_lookup_local(symbol_scope_t *scope, const char *name) {
  if (!scope || !name)
    return NULL;

  symbol_t *symbol = scope->symbols;
  while (symbol) {
    if (strcmp(symbol->name, name) == 0) {
      return symbol;
    }
    symbol = symbol->next;
  }

  return NULL; // Symbol not found in this scope
}

/* Print the symbol table (for debugging) */
static const char *symbol_kind_names[] = {"TYPE", "FUNCTION", "VAR",
                                          "PARAMETER", "CLASS"};

void print_symbol_table(symbol_scope_t *scope, int depth) {
  if (!scope)
    return;

  // Print scope indentation
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
  printf("Scope:\n");

  // Print symbols in this scope
  symbol_t *symbol = scope->symbols;
  while (symbol) {
    for (int i = 0; i < depth + 1; i++) {
      printf("  ");
    }

    printf("%s: %s\n", symbol->name, symbol_kind_names[symbol->kind]);
    symbol = symbol->next;
  }
}
