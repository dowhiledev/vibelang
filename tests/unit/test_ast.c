#include "../../src/utils/ast.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test AST node creation and property setting
static void test_ast_creation() {
  // Create a function declaration node
  ast_node_t *func = create_ast_node(AST_FUNCTION_DECL);
  assert(func != NULL);
  assert(func->type == AST_FUNCTION_DECL);

  // Set properties
  ast_set_string(func, "name", "testFunction");
  ast_set_int(func, "line", 10);

  // Verify properties
  assert(strcmp(ast_get_string(func, "name"), "testFunction") == 0);
  assert(ast_get_int(func, "line") == 10);

  // Create parameter node
  ast_node_t *param = create_ast_node(AST_PARAMETER);
  ast_set_string(param, "name", "x");

  // Create type node
  ast_node_t *type = create_ast_node(AST_BASIC_TYPE);
  ast_set_string(type, "type", "Int");

  // Add type as child of parameter
  ast_add_child(param, type);

  // Add parameter as child of function
  ast_add_child(func, param);

  // Verify children
  assert(func->child_count == 1);
  assert(func->children[0] == param);
  assert(param->child_count == 1);
  assert(param->children[0] == type);

  // Clean up
  ast_node_free(func);

  printf("AST creation test passed\n");
}

// Test property overwriting
static void test_property_overwrite() {
  ast_node_t *node = create_ast_node(AST_VAR_DECL);

  // Set initial value
  ast_set_string(node, "name", "oldName");
  assert(strcmp(ast_get_string(node, "name"), "oldName") == 0);

  // Overwrite value
  ast_set_string(node, "name", "newName");
  assert(strcmp(ast_get_string(node, "name"), "newName") == 0);

  // Change property type
  ast_set_int(node, "name", 42);
  assert(ast_get_int(node, "name") == 42);

  // Clean up
  ast_node_free(node);

  printf("Property overwrite test passed\n");
}

// Test different property types
static void test_property_types() {
  ast_node_t *node = create_ast_node(AST_EXPR_STMT);

  // Set different property types
  ast_set_int(node, "int_prop", 42);
  ast_set_float(node, "float_prop", 3.14159);
  ast_set_string(node, "string_prop", "hello");
  ast_set_bool(node, "bool_prop", 1);

  // Get properties
  assert(ast_get_int(node, "int_prop") == 42);
  assert(ast_get_float(node, "float_prop") == 3.14159);
  assert(strcmp(ast_get_string(node, "string_prop"), "hello") == 0);
  assert(ast_get_bool(node, "bool_prop") == 1);

  // Try getting non-existent property
  assert(ast_get_int(node, "nonexistent") == 0);
  assert(ast_get_float(node, "nonexistent") == 0.0);
  assert(ast_get_string(node, "nonexistent") == NULL);
  assert(ast_get_bool(node, "nonexistent") == 0);

  // Clean up
  ast_node_free(node);

  printf("Property types test passed\n");
}

// Test building a complex AST
static void test_complex_ast() {
  // Create a program node
  ast_node_t *program = create_ast_node(AST_PROGRAM);

  // Add a function declaration
  ast_node_t *func = create_ast_node(AST_FUNCTION_DECL);
  ast_set_string(func, "name", "calculate");
  ast_add_child(program, func);

  // Create parameter list
  ast_node_t *params = create_ast_node(AST_PARAM_LIST);
  ast_add_child(func, params);

  // Add parameters
  ast_node_t *param1 = create_ast_node(AST_PARAMETER);
  ast_set_string(param1, "name", "a");
  ast_node_t *type1 = create_ast_node(AST_BASIC_TYPE);
  ast_set_string(type1, "type", "Int");
  ast_add_child(param1, type1);
  ast_add_child(params, param1);

  ast_node_t *param2 = create_ast_node(AST_PARAMETER);
  ast_set_string(param2, "name", "b");
  ast_node_t *type2 = create_ast_node(AST_BASIC_TYPE);
  ast_set_string(type2, "type", "Int");
  ast_add_child(param2, type2);
  ast_add_child(params, param2);

  // Create function body
  ast_node_t *body = create_ast_node(AST_FUNCTION_BODY);
  ast_add_child(func, body);

  // Add variable declaration
  ast_node_t *var_decl = create_ast_node(AST_VAR_DECL);
  ast_set_string(var_decl, "name", "result");
  ast_node_t *init_expr = create_ast_node(AST_INT_LITERAL);
  ast_set_int(init_expr, "value", 0);
  ast_add_child(var_decl, init_expr);
  ast_add_child(body, var_decl);

  // Add return statement
  ast_node_t *ret_stmt = create_ast_node(AST_RETURN_STMT);
  ast_node_t *id_expr = create_ast_node(AST_IDENTIFIER);
  ast_set_string(id_expr, "name", "result");
  ast_add_child(ret_stmt, id_expr);
  ast_add_child(body, ret_stmt);

  // Verify structure
  assert(program->child_count == 1);
  assert(program->children[0] == func);
  assert(func->child_count == 2);
  assert(params->child_count == 2);
  assert(body->child_count == 2);

  // Clean up
  ast_node_free(program);

  printf("Complex AST test passed\n");
}

int main() {
  printf("Running AST tests...\n");

  test_ast_creation();
  test_property_overwrite();
  test_property_types();
  test_complex_ast();

  printf("All AST tests passed!\n");
  return 0;
}
