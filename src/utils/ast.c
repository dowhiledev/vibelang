#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "log_utils.h"

#define INITIAL_CAPACITY 8

const char* ast_type_names[] = {
    "PROGRAM", "IMPORT", "TYPE_DECL", "MEANING_TYPE", "BASIC_TYPE",
    "FUNCTION_DECL", "PARAM_LIST", "PARAMETER", "FUNCTION_BODY", 
    "PROMPT_BLOCK", "CLASS_DECL", "MEMBER_VAR", "VAR_DECL", 
    "EXPR_STMT", "RETURN_STMT", "BLOCK", "IDENTIFIER", 
    "INT_LITERAL", "FLOAT_LITERAL", "STRING_LITERAL", "BOOL_LITERAL",
    "CALL_EXPR"
};

// Static counters for monitoring AST creation
static int ast_current_depth = 0;
static int ast_node_count = 0;

// Reset AST metrics at the beginning of parsing
void ast_reset_metrics() {
    ast_current_depth = 0;
    ast_node_count = 0;
}

// Get current AST metrics for diagnostics
void ast_get_metrics(int* depth, int* count) {
    if (depth) *depth = ast_current_depth;
    if (count) *count = ast_node_count;
}

/* AST Creation and Management */
ast_node_t* create_ast_node(ast_node_type_t type) {
    // Check if we're exceeding safety limits
    if (ast_node_count >= MAX_AST_NODES) {
        fprintf(stderr, "ERROR: Maximum AST node limit reached (%d nodes)\n", 
                MAX_AST_NODES);
        return NULL;  // Abort by returning NULL
    }
    
    // Increment counter for each node created
    ast_node_count++;
    
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
    
    // Check if we're exceeding depth limits before adding a child
    if (ast_current_depth >= MAX_AST_DEPTH) {
        fprintf(stderr, "ERROR: Maximum AST depth limit reached (%d levels)\n", 
                MAX_AST_DEPTH);
        return;  // Abort by not adding the child
    }
    
    ast_current_depth++;  // Increment depth before adding
    
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
    
    ast_current_depth--;  // Decrement after adding
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

/**
 * Helper function to get AST node type name as string
 */
static const char* ast_type_to_string(ast_node_type_t type) {
    switch (type) {
        case AST_PROGRAM: return "PROGRAM";
        case AST_FUNCTION_DECL: return "FUNCTION_DECL";
        case AST_FUNCTION_BODY: return "FUNCTION_BODY";
        case AST_PARAM_LIST: return "PARAM_LIST";
        case AST_PARAMETER: return "PARAMETER";
        case AST_TYPE_DECL: return "TYPE_DECL";
        case AST_BASIC_TYPE: return "BASIC_TYPE";
        case AST_MEANING_TYPE: return "MEANING_TYPE";
        case AST_CLASS_DECL: return "CLASS_DECL";
        case AST_MEMBER_VAR: return "MEMBER_VAR";
        case AST_IMPORT: return "IMPORT";
        case AST_BLOCK: return "BLOCK";
        case AST_VAR_DECL: return "VAR_DECL";
        case AST_RETURN_STMT: return "RETURN_STMT";
        case AST_PROMPT_BLOCK: return "PROMPT_BLOCK";
        case AST_EXPR_STMT: return "EXPR_STMT";
        case AST_CALL_EXPR: return "CALL_EXPR";
        case AST_STRING_LITERAL: return "STRING_LITERAL";
        case AST_INT_LITERAL: return "INT_LITERAL";
        case AST_FLOAT_LITERAL: return "FLOAT_LITERAL";
        case AST_BOOL_LITERAL: return "BOOL_LITERAL";
        case AST_IDENTIFIER: return "IDENTIFIER";
        default: return "UNKNOWN";
    }
}

/**
 * Helper function to print indentation
 */
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

/**
 * Print AST node structure with indentation
 */
void ast_print(const ast_node_t* node, int indent) {
    if (!node) {
        print_indent(indent);
        printf("(NULL)\n");
        return;
    }

    print_indent(indent);
    printf("%s", ast_type_to_string(node->type));
    
    // Print properties based on node type
    for (size_t i = 0; i < node->prop_count; i++) {
        printf(" %s=", node->props[i].key);
        switch (node->props[i].type) {
            case AST_VAL_INT:
                printf("%lld", node->props[i].val.int_val);
                break;
            case AST_VAL_FLOAT:
                printf("%.6f", node->props[i].val.float_val);
                break;
            case AST_VAL_BOOL:
                printf("%s", node->props[i].val.bool_val ? "true" : "false");
                break;
            case AST_VAL_STRING:
                printf("\"%s\"", node->props[i].val.str_val);
                break;
            case AST_VAL_POINTER:
                printf("ptr:%p", node->props[i].val.ptr_val);
                break;
            default:
                printf("(unknown type)");
                break;
        }
    }
    printf("\n");
    
    // Print children
    for (size_t i = 0; i < node->child_count; i++) {
        ast_print(node->children[i], indent + 1);
    }
}
