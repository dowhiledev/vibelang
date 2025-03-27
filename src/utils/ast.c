#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../utils/ast.h"

#define INITIAL_CAPACITY 8

const char* ast_type_names[] = {
    "PROGRAM", "IMPORT", "TYPE_DECL", "MEANING_TYPE", "BASIC_TYPE",
    "FUNCTION_DECL", "PARAM_LIST", "PARAMETER", "FUNCTION_BODY", 
    "PROMPT_BLOCK", "CLASS_DECL", "MEMBER_VAR", "VAR_DECL", 
    "EXPR_STMT", "RETURN_STMT", "BLOCK", "IDENTIFIER", 
    "INT_LITERAL", "FLOAT_LITERAL", "STRING_LITERAL", "BOOL_LITERAL",
    "CALL_EXPR"
};

/* AST Creation and Management */
ast_node_t* create_ast_node(ast_node_type_t type) {
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for AST node\n");
        return NULL;
    }
    
    node->type = type;
    
    node->props = NULL;
    node->prop_count = 0;
    node->prop_capacity = 0;
    
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    
    node->line = 0;
    node->column = 0;
    
    return node;
}

void ast_node_free(ast_node_t* node) {
    if (!node) return;
    
    // Free properties
    for (size_t i = 0; i < node->prop_count; i++) {
        free(node->props[i].key);
        if (node->props[i].type == AST_VAL_STRING) {
            free(node->props[i].val.str_val);
        }
    }
    free(node->props);
    
    // Free children
    for (size_t i = 0; i < node->child_count; i++) {
        ast_node_free(node->children[i]);
    }
    free(node->children);
    
    free(node);
}

void ast_add_child(ast_node_t* parent, ast_node_t* child) {
    if (!parent || !child) return;
    
    // Expand capacity if needed
    if (parent->child_count == parent->child_capacity) {
        size_t new_capacity = parent->child_capacity == 0 ? INITIAL_CAPACITY : parent->child_capacity * 2;
        ast_node_t** new_children = (ast_node_t**)realloc(parent->children, new_capacity * sizeof(ast_node_t*));
        if (!new_children) {
            fprintf(stderr, "Memory allocation failed for AST children\n");
            return;
        }
        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }
    
    parent->children[parent->child_count++] = child;
}

/* Helper function to find or create a property */
static size_t ast_find_or_create_prop(ast_node_t* node, const char* key) {
    // Look for existing property
    for (size_t i = 0; i < node->prop_count; i++) {
        if (strcmp(node->props[i].key, key) == 0) {
            return i;
        }
    }
    
    // Need to create a new property
    if (node->prop_count == node->prop_capacity) {
        size_t new_capacity = node->prop_capacity == 0 ? INITIAL_CAPACITY : node->prop_capacity * 2;
        void* new_props = realloc(node->props, new_capacity * sizeof(*node->props));
        if (!new_props) {
            fprintf(stderr, "Memory allocation failed for AST properties\n");
            return (size_t)-1;
        }
        node->props = new_props;
        node->prop_capacity = new_capacity;
    }
    
    size_t index = node->prop_count++;
    node->props[index].key = strdup(key);
    return index;
}

/* Property setters */
void ast_set_int(ast_node_t* node, const char* key, long long value) {
    if (!node || !key) return;
    
    size_t index = ast_find_or_create_prop(node, key);
    if (index == (size_t)-1) return;
    
    // Clean up existing string value if needed
    if (node->props[index].type == AST_VAL_STRING) {
        free(node->props[index].val.str_val);
    }
    
    node->props[index].type = AST_VAL_INT;
    node->props[index].val.int_val = value;
}

void ast_set_float(ast_node_t* node, const char* key, double value) {
    if (!node || !key) return;
    
    size_t index = ast_find_or_create_prop(node, key);
    if (index == (size_t)-1) return;
    
    // Clean up existing string value if needed
    if (node->props[index].type == AST_VAL_STRING) {
        free(node->props[index].val.str_val);
    }
    
    node->props[index].type = AST_VAL_FLOAT;
    node->props[index].val.float_val = value;
}

void ast_set_string(ast_node_t* node, const char* key, const char* value) {
    if (!node || !key) return;
    
    size_t index = ast_find_or_create_prop(node, key);
    if (index == (size_t)-1) return;
    
    // Clean up existing string value if needed
    if (node->props[index].type == AST_VAL_STRING) {
        free(node->props[index].val.str_val);
    }
    
    node->props[index].type = AST_VAL_STRING;
    node->props[index].val.str_val = strdup(value);
}

void ast_set_bool(ast_node_t* node, const char* key, int value) {
    if (!node || !key) return;
    
    size_t index = ast_find_or_create_prop(node, key);
    if (index == (size_t)-1) return;
    
    // Clean up existing string value if needed
    if (node->props[index].type == AST_VAL_STRING) {
        free(node->props[index].val.str_val);
    }
    
    node->props[index].type = AST_VAL_BOOL;
    node->props[index].val.bool_val = value;
}

/* Property getters */
static size_t ast_find_prop(const ast_node_t* node, const char* key) {
    for (size_t i = 0; i < node->prop_count; i++) {
        if (strcmp(node->props[i].key, key) == 0) {
            return i;
        }
    }
    return (size_t)-1;
}

long long ast_get_int(const ast_node_t* node, const char* key) {
    if (!node || !key) return 0;
    
    size_t index = ast_find_prop(node, key);
    if (index == (size_t)-1 || node->props[index].type != AST_VAL_INT) {
        return 0;
    }
    
    return node->props[index].val.int_val;
}

double ast_get_float(const ast_node_t* node, const char* key) {
    if (!node || !key) return 0.0;
    
    size_t index = ast_find_prop(node, key);
    if (index == (size_t)-1 || node->props[index].type != AST_VAL_FLOAT) {
        return 0.0;
    }
    
    return node->props[index].val.float_val;
}

const char* ast_get_string(const ast_node_t* node, const char* key) {
    if (!node || !key) return NULL;
    
    size_t index = ast_find_prop(node, key);
    if (index == (size_t)-1 || node->props[index].type != AST_VAL_STRING) {
        return NULL;
    }
    
    return node->props[index].val.str_val;
}

int ast_get_bool(const ast_node_t* node, const char* key) {
    if (!node || !key) return 0;
    
    size_t index = ast_find_prop(node, key);
    if (index == (size_t)-1 || node->props[index].type != AST_VAL_BOOL) {
        return 0;
    }
    
    return node->props[index].val.bool_val;
}

/* Debug utility to print AST */
void ast_print(const ast_node_t* node, int depth) {
    if (!node) return;
    
    // Print indentation
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    // Print node type
    printf("%s", ast_type_names[node->type]);
    
    // Print properties
    for (size_t i = 0; i < node->prop_count; i++) {
        printf(" %s=", node->props[i].key);
        switch (node->props[i].type) {
            case AST_VAL_INT:
                printf("%lld", node->props[i].val.int_val);
                break;
            case AST_VAL_FLOAT:
                printf("%f", node->props[i].val.float_val);
                break;
            case AST_VAL_STRING:
                printf("\"%s\"", node->props[i].val.str_val);
                break;
            case AST_VAL_BOOL:
                printf("%s", node->props[i].val.bool_val ? "true" : "false");
                break;
            default:
                printf("(undefined)");
                break;
        }
    }
    printf("\n");
    
    // Print children
    for (size_t i = 0; i < node->child_count; i++) {
        ast_print(node->children[i], depth + 1);
    }
}
