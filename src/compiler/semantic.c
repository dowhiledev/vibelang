#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/ast.h"
#include "../utils/log_utils.h"
#include "../../include/symbol_table.h"

/* Forward declarations */
static int analyze_node(ast_node_t* node, symbol_scope_t* scope);
static int analyze_program(ast_node_t* node, symbol_scope_t* scope);
static int analyze_import(ast_node_t* node, symbol_scope_t* scope);
static int analyze_type_decl(ast_node_t* node, symbol_scope_t* scope);
static int analyze_function_decl(ast_node_t* node, symbol_scope_t* scope);
static int analyze_class_decl(ast_node_t* node, symbol_scope_t* scope);
static int analyze_function_body(ast_node_t* node, symbol_scope_t* scope);
static int analyze_var_decl(ast_node_t* node, symbol_scope_t* scope);
static int analyze_expr_stmt(ast_node_t* node, symbol_scope_t* scope);
static int analyze_return_stmt(ast_node_t* node, symbol_scope_t* scope);
static int analyze_block(ast_node_t* node, symbol_scope_t* scope);
static int analyze_expression(ast_node_t* node, symbol_scope_t* scope);
static int analyze_call_expr(ast_node_t* node, symbol_scope_t* scope);

/* Type compatibility checking */
static ast_node_t* get_expression_type(ast_node_t* expr, symbol_scope_t* scope);
static int is_type_compatible(ast_node_t* expected, ast_node_t* actual);
static int is_basic_type_compatible(ast_node_t* expected, ast_node_t* actual);

/* Global symbol table */
static symbol_scope_t* global_scope = NULL;
static int error_count = 0;

/* Main entry point for semantic analysis */
int semantic_analyze(ast_node_t* ast) {
    if (!ast) return 0;
    
    // Create the global scope
    global_scope = create_symbol_scope(NULL, ast);
    if (!global_scope) {
        ERROR("Failed to create global symbol scope");
        return 0;
    }
    
    error_count = 0;
    int result = analyze_node(ast, global_scope);
    
    if (error_count > 0) {
        ERROR("Semantic analysis failed with %d errors", error_count);
        return 0;
    }
    
    INFO("Semantic analysis completed successfully");
    return result;
}

/* Clean up resources */
void semantic_cleanup(void) {
    if (global_scope) {
        free_symbol_scope(global_scope);
        global_scope = NULL;
    }
}

/* Analyze an AST node based on its type */
static int analyze_node(ast_node_t* node, symbol_scope_t* scope) {
    if (!node) return 1;
    
    switch (node->type) {
        case AST_PROGRAM:
            return analyze_program(node, scope);
        case AST_IMPORT:
            return analyze_import(node, scope);
        case AST_TYPE_DECL:
            return analyze_type_decl(node, scope);
        case AST_FUNCTION_DECL:
            return analyze_function_decl(node, scope);
        case AST_CLASS_DECL:
            return analyze_class_decl(node, scope);
        case AST_FUNCTION_BODY:
            return analyze_function_body(node, scope);
        case AST_VAR_DECL:
            return analyze_var_decl(node, scope);
        case AST_EXPR_STMT:
            return analyze_expr_stmt(node, scope);
        case AST_RETURN_STMT:
            return analyze_return_stmt(node, scope);
        case AST_BLOCK:
            return analyze_block(node, scope);
        case AST_CALL_EXPR:
            return analyze_call_expr(node, scope);
        default:
            // Skip nodes that don't need semantic analysis
            return 1;
    }
}

/* Analyze program node */
static int analyze_program(ast_node_t* node, symbol_scope_t* scope) {
    // Process all children (declarations)
    for (size_t i = 0; i < node->child_count; i++) {
        if (!analyze_node(node->children[i], scope)) {
            return 0;
        }
    }
    return 1;
}

/* Analyze import statement */
static int analyze_import(ast_node_t* node, symbol_scope_t* scope) {
    // Here we would handle importing symbols from other modules
    // For now, just a placeholder
    const char* path = ast_get_string(node, "path");
    if (!path) {
        ERROR("Import statement missing path");
        error_count++;
        return 0;
    }
    
    INFO("Import '%s' found (processing imports not yet implemented)", path);
    return 1;
}

/* Analyze type declaration */
static int analyze_type_decl(ast_node_t* node, symbol_scope_t* scope) {
    const char* name = ast_get_string(node, "name");
    if (!name) {
        ERROR("Type declaration missing name");
        error_count++;
        return 0;
    }
    
    // Add type to symbol table
    symbol_t* symbol = symbol_add(scope, name, SYM_TYPE, node, node->children[0]);
    if (!symbol) {
        ERROR("Failed to add type '%s' to symbol table", name);
        error_count++;
        return 0;
    }
    
    return 1;
}

/* Analyze function declaration */
static int analyze_function_decl(ast_node_t* node, symbol_scope_t* scope) {
    const char* name = ast_get_string(node, "name");
    if (!name) {
        ERROR("Function declaration missing name");
        error_count++;
        return 0;
    }
    
    // Find return type (if specified)
    ast_node_t* return_type = NULL;
    ast_node_t* body = NULL;
    
    // Check for param list and return type
    for (size_t i = 0; i < node->child_count; i++) {
        ast_node_t* child = node->children[i];
        if (child->type == AST_FUNCTION_BODY) {
            body = child;
        } else if (child->type == AST_BASIC_TYPE || child->type == AST_MEANING_TYPE) {
            return_type = child;
        }
    }
    
    // Add function to symbol table
    symbol_t* symbol = symbol_add(scope, name, SYM_FUNCTION, node, return_type);
    if (!symbol) {
        ERROR("Failed to add function '%s' to symbol table", name);
        error_count++;
        return 0;
    }
    
    // Create a new scope for the function
    symbol_scope_t* function_scope = create_symbol_scope(scope, node);
    if (!function_scope) {
        ERROR("Failed to create scope for function '%s'", name);
        error_count++;
        return 0;
    }
    
    // Process parameter list
    for (size_t i = 0; i < node->child_count; i++) {
        ast_node_t* child = node->children[i];
        if (child->type == AST_PARAM_LIST) {
            // Add each parameter to the function scope
            for (size_t j = 0; j < child->child_count; j++) {
                ast_node_t* param = child->children[j];
                const char* param_name = ast_get_string(param, "name");
                if (param_name) {
                    // Get parameter type
                    ast_node_t* param_type = NULL;
                    if (param->child_count > 0) {
                        param_type = param->children[0];
                    }
                    
                    symbol_t* param_symbol = symbol_add(function_scope, param_name, 
                                                       SYM_PARAMETER, param, param_type);
                    if (!param_symbol) {
                        ERROR("Failed to add parameter '%s' to function '%s'", param_name, name);
                        error_count++;
                    }
                }
            }
        }
    }
    
    // Process function body
    if (body) {
        if (!analyze_function_body(body, function_scope)) {
            return 0;
        }
    }
    
    // Clean up function scope
    free_symbol_scope(function_scope);
    
    return 1;
}

/* Analyze class declaration */
static int analyze_class_decl(ast_node_t* node, symbol_scope_t* scope) {
    const char* name = ast_get_string(node, "name");
    if (!name) {
        ERROR("Class declaration missing name");
        error_count++;
        return 0;
    }
    
    // Add class to symbol table
    symbol_t* symbol = symbol_add(scope, name, SYM_CLASS, node, NULL);
    if (!symbol) {
        ERROR("Failed to add class '%s' to symbol table", name);
        error_count++;
        return 0;
    }
    
    // Create a new scope for the class
    symbol_scope_t* class_scope = create_symbol_scope(scope, node);
    if (!class_scope) {
        ERROR("Failed to create scope for class '%s'", name);
        error_count++;
        return 0;
    }
    
    // Process class members
    for (size_t i = 0; i < node->child_count; i++) {
        ast_node_t* child = node->children[i];
        
        if (child->type == AST_MEMBER_VAR) {
            const char* member_name = ast_get_string(child, "name");
            if (member_name) {
                // Get member type
                ast_node_t* member_type = NULL;
                if (child->child_count > 0) {
                    member_type = child->children[0];
                }
                
                symbol_t* member_symbol = symbol_add(class_scope, member_name, 
                                                   SYM_VAR, child, member_type);
                if (!member_symbol) {
                    ERROR("Failed to add member variable '%s' to class '%s'", member_name, name);
                    error_count++;
                }
            }
        } else if (child->type == AST_FUNCTION_DECL) {
            // Process method declaration
            if (!analyze_function_decl(child, class_scope)) {
                return 0;
            }
        }
    }
    
    // Clean up class scope
    free_symbol_scope(class_scope);
    
    return 1;
}

/* Analyze function body */
static int analyze_function_body(ast_node_t* node, symbol_scope_t* scope) {
    // Process statements
    for (size_t i = 0; i < node->child_count; i++) {
        if (!analyze_node(node->children[i], scope)) {
            return 0;
        }
    }
    
    return 1;
}

/* Analyze variable declaration */
static int analyze_var_decl(ast_node_t* node, symbol_scope_t* scope) {
    const char* name = ast_get_string(node, "name");
    if (!name) {
        ERROR("Variable declaration missing name");
        error_count++;
        return 0;
    }
    
    // Get type (if explicitly specified)
    ast_node_t* var_type = NULL;
    ast_node_t* init_expr = NULL;
    
    for (size_t i = 0; i < node->child_count; i++) {
        ast_node_t* child = node->children[i];
        if (child->type == AST_BASIC_TYPE || child->type == AST_MEANING_TYPE) {
            var_type = child;
        } else {
            init_expr = child;  // Initialization expression
        }
    }
    
    // If no explicit type, infer from initialization expression
    if (!var_type && init_expr) {
        var_type = get_expression_type(init_expr, scope);
    }
    
    if (!var_type) {
        ERROR("Cannot determine type for variable '%s'", name);
        error_count++;
        return 0;
    }
    
    // Add variable to symbol table
    symbol_t* symbol = symbol_add(scope, name, SYM_VAR, node, var_type);
    if (!symbol) {
        ERROR("Failed to add variable '%s' to symbol table", name);
        error_count++;
        return 0;
    }
    
    return 1;
}

/* Analyze expression statement */
static int analyze_expr_stmt(ast_node_t* node, symbol_scope_t* scope) {
    if (node->child_count == 0) {
        WARNING("Empty expression statement");
        return 1;
    }
    
    // Analyze the expression
    return analyze_expression(node->children[0], scope);
}

/* Analyze return statement */
static int analyze_return_stmt(ast_node_t* node, symbol_scope_t* scope) {
    // TODO: Check return type compatibility with function return type
    
    // If there is an expression, analyze it
    if (node->child_count > 0) {
        return analyze_expression(node->children[0], scope);
    }
    
    return 1;
}

/* Analyze block */
static int analyze_block(ast_node_t* node, symbol_scope_t* scope) {
    // Create a new scope for the block
    symbol_scope_t* block_scope = create_symbol_scope(scope, node);
    if (!block_scope) {
        ERROR("Failed to create scope for block");
        error_count++;
        return 0;
    }
    
    // Process statements
    for (size_t i = 0; i < node->child_count; i++) {
        if (!analyze_node(node->children[i], block_scope)) {
            free_symbol_scope(block_scope);
            return 0;
        }
    }
    
    // Clean up block scope
    free_symbol_scope(block_scope);
    
    return 1;
}

/* Analyze general expression */
static int analyze_expression(ast_node_t* node, symbol_scope_t* scope) {
    if (!node) return 1;
    
    switch (node->type) {
        case AST_CALL_EXPR:
            return analyze_call_expr(node, scope);
        case AST_IDENTIFIER: {
            const char* name = ast_get_string(node, "name");
            if (name) {
                symbol_t* symbol = symbol_lookup(scope, name);
                if (!symbol) {
                    ERROR("Undefined identifier: %s", name);
                    error_count++;
                    return 0;
                }
            }
            return 1;
        }
        // Literals are always valid
        case AST_INT_LITERAL:
        case AST_FLOAT_LITERAL:
        case AST_STRING_LITERAL:
        case AST_BOOL_LITERAL:
            return 1;
        default:
            return 1;
    }
}

/* Analyze function call expression */
static int analyze_call_expr(ast_node_t* node, symbol_scope_t* scope) {
    const char* func_name = ast_get_string(node, "function");
    if (!func_name) {
        ERROR("Function call missing function name");
        error_count++;
        return 0;
    }
    
    // Look up function in symbol table
    symbol_t* func_symbol = symbol_lookup(scope, func_name);
    if (!func_symbol) {
        ERROR("Call to undefined function: %s", func_name);
        error_count++;
        return 0;
    }
    
    if (func_symbol->kind != SYM_FUNCTION) {
        ERROR("'%s' is not a function", func_name);
        error_count++;
        return 0;
    }
    
    // Get parameter list from function declaration
    ast_node_t* param_list = NULL;
    for (size_t i = 0; i < func_symbol->node->child_count; i++) {
        if (func_symbol->node->children[i]->type == AST_PARAM_LIST) {
            param_list = func_symbol->node->children[i];
            break;
        }
    }
    
    // Check argument count
    size_t param_count = param_list ? param_list->child_count : 0;
    size_t arg_count = node->child_count;
    
    if (arg_count != param_count) {
        ERROR("Function '%s' called with wrong number of arguments (expected %zu, got %zu)",
              func_name, param_count, arg_count);
        error_count++;
        return 0;
    }
    
    // Check each argument's type compatibility
    for (size_t i = 0; i < arg_count; i++) {
        ast_node_t* arg = node->children[i];
        ast_node_t* param = param_list->children[i];
        ast_node_t* expected_type = param->children[0];  // Parameter type
        ast_node_t* actual_type = get_expression_type(arg, scope);
        
        if (actual_type && !is_type_compatible(expected_type, actual_type)) {
            const char* param_name = ast_get_string(param, "name");
            ERROR("Type mismatch for argument %zu in call to '%s' (parameter '%s')",
                  i + 1, func_name, param_name);
            error_count++;
            return 0;
        }
    }
    
    return 1;
}

/* Get the type of an expression */
static ast_node_t* get_expression_type(ast_node_t* expr, symbol_scope_t* scope) {
    if (!expr) return NULL;
    
    switch (expr->type) {
        case AST_INT_LITERAL: {
            ast_node_t* type = create_ast_node(AST_BASIC_TYPE);
            ast_set_string(type, "type", "Int");
            return type;
        }
        case AST_FLOAT_LITERAL: {
            ast_node_t* type = create_ast_node(AST_BASIC_TYPE);
            ast_set_string(type, "type", "Float");
            return type;
        }
        case AST_STRING_LITERAL: {
            ast_node_t* type = create_ast_node(AST_BASIC_TYPE);
            ast_set_string(type, "type", "String");
            return type;
        }
        case AST_BOOL_LITERAL: {
            ast_node_t* type = create_ast_node(AST_BASIC_TYPE);
            ast_set_string(type, "type", "Bool");
            return type;
        }
        case AST_IDENTIFIER: {
            const char* name = ast_get_string(expr, "name");
            if (name) {
                symbol_t* symbol = symbol_lookup(scope, name);
                if (symbol && symbol->type_node) {
                    return symbol->type_node;
                }
            }
            return NULL;
        }
        case AST_CALL_EXPR: {
            const char* func_name = ast_get_string(expr, "function");
            if (func_name) {
                symbol_t* symbol = symbol_lookup(scope, func_name);
                if (symbol && symbol->kind == SYM_FUNCTION) {
                    return symbol->type_node;  // Return type of the function
                }
            }
            return NULL;
        }
        default:
            return NULL;
    }
}

/* Check if two types are compatible */
static int is_type_compatible(ast_node_t* expected, ast_node_t* actual) {
    if (!expected || !actual) return 0;
    
    // Handle different types of type nodes
    if (expected->type == AST_BASIC_TYPE && actual->type == AST_BASIC_TYPE) {
        return is_basic_type_compatible(expected, actual);
    } else if (expected->type == AST_MEANING_TYPE && actual->type == AST_MEANING_TYPE) {
        // For Meaning types, check the underlying type
        if (expected->child_count > 0 && actual->child_count > 0) {
            return is_type_compatible(expected->children[0], actual->children[0]);
        }
        return 0;
    } else if (expected->type == AST_MEANING_TYPE && actual->type == AST_BASIC_TYPE) {
        // Meaning type can accept basic type if they match
        if (expected->child_count > 0) {
            return is_type_compatible(expected->children[0], actual);
        }
        return 0;
    }
    
    return 0;
}

/* Check if two basic types are compatible */
static int is_basic_type_compatible(ast_node_t* expected, ast_node_t* actual) {
    const char* expected_type = ast_get_string(expected, "type");
    const char* actual_type = ast_get_string(actual, "type");
    
    if (!expected_type || !actual_type) return 0;
    
    // Exact type match
    if (strcmp(expected_type, actual_type) == 0) {
        return 1;
    }
    
    // Numeric type conversion allowed in some cases
    if (strcmp(expected_type, "Float") == 0 && strcmp(actual_type, "Int") == 0) {
        return 1;  // Int can be converted to Float
    }
    
    return 0;
}
