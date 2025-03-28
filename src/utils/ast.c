#include "ast.h"
#include "log_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Initial capacity for children and properties arrays
#define INITIAL_CAPACITY 8

// Global metrics for tracking AST stats
static int ast_max_depth = 0;
static int ast_current_depth = 0;
static int ast_node_count = 0;

void ast_reset_metrics() {
    ast_max_depth = 0;
    ast_current_depth = 0;
    ast_node_count = 0;
}

void ast_get_metrics(int* depth, int* count) {
    if (depth) *depth = ast_max_depth;
    if (count) *count = ast_node_count;
}

ast_node_t* create_ast_node(ast_node_type_t type) {
    // Check if we're approaching resource limits
    if (ast_node_count >= MAX_AST_NODES) {
        ERROR("AST node limit exceeded (%d). Possible infinite recursion?",
                MAX_AST_NODES);
        return NULL;
    }
    
    ast_node_t* node = (ast_node_t*)malloc(sizeof(ast_node_t));
    if (!node) {
        ERROR("Failed to allocate memory for AST node");
        return NULL;
    }
    
    node->type = type;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    node->properties = NULL;
    node->line = 0;
    node->column = 0;
    node->parent = NULL;
    
    // Increment node count for metrics
    ast_node_count++;
    
    return node;
}

void ast_node_free(ast_node_t* node) {
    if (!node) return;
    
    // Free all children first
    for (int i = 0; i < node->child_count; i++) {
        ast_node_free(node->children[i]);
    }
    free(node->children);
    
    // Free all properties
    ast_prop_t* prop = node->properties;
    while (prop) {
        ast_prop_t* next = prop->next;
        free(prop->name);
        if (prop->type == AST_PROP_STRING && prop->str_val) {
            free(prop->str_val);
        }
        free(prop);
        prop = next;
    }
    
    // Free the node itself
    free(node);
}

void ast_add_child(ast_node_t* parent, ast_node_t* child) {
    if (!parent || !child) return;
    
    // Check if we're approaching max depth
    if (ast_current_depth >= MAX_AST_DEPTH) {
        ERROR("AST depth limit exceeded (%d). Possible infinite recursion?",
                MAX_AST_DEPTH);
        return;
    }
    
    // Expand children array if needed
    if (parent->child_count == parent->child_capacity) {
        int new_capacity = parent->child_capacity == 0 ? INITIAL_CAPACITY : parent->child_capacity * 2;
        ast_node_t** new_children = (ast_node_t**)realloc(parent->children, new_capacity * sizeof(ast_node_t*));
        if (!new_children) {
            ERROR("Failed to reallocate memory for AST node children");
            return;
        }
        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }
    
    // Add child and update parent reference
    parent->children[parent->child_count++] = child;
    child->parent = parent;
    
    // Update metrics
    ast_current_depth++;
    if (ast_current_depth > ast_max_depth) {
        ast_max_depth = ast_current_depth;
    }
    ast_current_depth--; // Restore depth counter after recursion
}

// Helper to find a property by name
static ast_prop_t* find_property(const ast_node_t* node, const char* name) {
    ast_prop_t* prop = node->properties;
    while (prop) {
        if (strcmp(prop->name, name) == 0) {
            return prop;
        }
        prop = prop->next;
    }
    return NULL;
}

void ast_set_string(ast_node_t* node, const char* name, const char* value) {
    if (!node || !name) return;
    
    // Try to find existing property first
    ast_prop_t* prop = find_property(node, name);
    
    if (prop) {
        // Update existing property
        if (prop->type == AST_PROP_STRING && prop->str_val) {
            free(prop->str_val);
        }
        prop->type = AST_PROP_STRING;
        prop->str_val = value ? strdup(value) : NULL;
    } else {
        // Create new property
        prop = (ast_prop_t*)malloc(sizeof(ast_prop_t));
        if (!prop) {
            ERROR("Failed to allocate memory for AST property");
            return;
        }
        
        prop->name = strdup(name);
        prop->type = AST_PROP_STRING;
        prop->str_val = value ? strdup(value) : NULL;
        
        // Add to linked list
        prop->next = node->properties;
        node->properties = prop;
    }
}

void ast_set_int(ast_node_t* node, const char* name, int64_t value) {
    if (!node || !name) return;
    
    ast_prop_t* prop = find_property(node, name);
    
    if (prop) {
        // Update existing property
        if (prop->type == AST_PROP_STRING && prop->str_val) {
            free(prop->str_val);
        }
        prop->type = AST_PROP_INT;
        prop->int_val = value;
    } else {
        // Create new property
        prop = (ast_prop_t*)malloc(sizeof(ast_prop_t));
        if (!prop) {
            ERROR("Failed to allocate memory for AST property");
            return;
        }
        
        prop->name = strdup(name);
        prop->type = AST_PROP_INT;
        prop->int_val = value;
        
        // Add to linked list
        prop->next = node->properties;
        node->properties = prop;
    }
}

void ast_set_float(ast_node_t* node, const char* name, double value) {
    if (!node || !name) return;
    
    ast_prop_t* prop = find_property(node, name);
    
    if (prop) {
        // Update existing property
        if (prop->type == AST_PROP_STRING && prop->str_val) {
            free(prop->str_val);
        }
        prop->type = AST_PROP_FLOAT;
        prop->float_val = value;
    } else {
        // Create new property
        prop = (ast_prop_t*)malloc(sizeof(ast_prop_t));
        if (!prop) {
            ERROR("Failed to allocate memory for AST property");
            return;
        }
        
        prop->name = strdup(name);
        prop->type = AST_PROP_FLOAT;
        prop->float_val = value;
        
        // Add to linked list
        prop->next = node->properties;
        node->properties = prop;
    }
}

void ast_set_bool(ast_node_t* node, const char* name, bool value) {
    if (!node || !name) return;
    
    ast_prop_t* prop = find_property(node, name);
    
    if (prop) {
        // Update existing property
        if (prop->type == AST_PROP_STRING && prop->str_val) {
            free(prop->str_val);
        }
        prop->type = AST_PROP_BOOL;
        prop->bool_val = value;
    } else {
        // Create new property
        prop = (ast_prop_t*)malloc(sizeof(ast_prop_t));
        if (!prop) {
            ERROR("Failed to allocate memory for AST property");
            return;
        }
        
        prop->name = strdup(name);
        prop->type = AST_PROP_BOOL;
        prop->bool_val = value;
        
        // Add to linked list
        prop->next = node->properties;
        node->properties = prop;
    }
}

const char* ast_get_string(const ast_node_t* node, const char* name) {
    if (!node || !name) return NULL;
    
    ast_prop_t* prop = find_property(node, name);
    if (prop && prop->type == AST_PROP_STRING) {
        return prop->str_val;
    }
    return NULL;
}

int64_t ast_get_int(const ast_node_t* node, const char* name) {
    if (!node || !name) return 0;
    
    ast_prop_t* prop = find_property(node, name);
    if (prop && prop->type == AST_PROP_INT) {
        return prop->int_val;
    }
    return 0;
}

double ast_get_float(const ast_node_t* node, const char* name) {
    if (!node || !name) return 0.0;
    
    ast_prop_t* prop = find_property(node, name);
    if (prop && prop->type == AST_PROP_FLOAT) {
        return prop->float_val;
    }
    return 0.0;
}

bool ast_get_bool(const ast_node_t* node, const char* name) {
    if (!node || !name) return false;
    
    ast_prop_t* prop = find_property(node, name);
    if (prop && prop->type == AST_PROP_BOOL) {
        return prop->bool_val;
    }
    return false;
}

void ast_remove_child(ast_node_t* parent, int index) {
    if (!parent || index < 0 || index >= parent->child_count) return;
    
    ast_node_free(parent->children[index]);
    
    // Shift remaining children
    for (int i = index; i < parent->child_count - 1; i++) {
        parent->children[i] = parent->children[i + 1];
    }
    
    parent->child_count--;
}

void ast_replace_child(ast_node_t* parent, int index, ast_node_t* new_child) {
    if (!parent || !new_child || index < 0 || index >= parent->child_count) return;
    
    ast_node_free(parent->children[index]);
    parent->children[index] = new_child;
    new_child->parent = parent;
}

// Maps AST node types to human-readable strings
const char* ast_node_type_name(ast_node_type_t type) {
    static const char* type_names[] = {
        "Program",
        "FunctionDeclaration",
        "FunctionBody",
        "TypeDeclaration",
        "VariableDeclaration",
        "BasicType",
        "MeaningType",
        "ParameterList",
        "Parameter",
        "ClassDeclaration",
        "ClassBody",
        "MemberVariable",
        "Import",
        "Block",
        "ExpressionStatement",
        "ReturnStatement",
        "PromptBlock",
        "StringLiteral",
        "IntLiteral",
        "FloatLiteral",
        "BoolLiteral",
        "Identifier",
        "CallExpression"
    };
    
    if (type >= 0 && type < sizeof(type_names) / sizeof(type_names[0])) {
        return type_names[type];
    }
    return "Unknown";
}

// Print AST node recursively for debugging
void ast_print(const ast_node_t* node) {
    static int indent = 0;
    
    if (!node) return;
    
    // Print indentation
    for (int i = 0; i < indent; i++) printf("  ");
    
    // Print node type
    printf("%s", ast_node_type_name(node->type));
    
    // Print properties
    ast_prop_t* prop = node->properties;
    bool first = true;
    
    while (prop) {
        if (first) {
            printf(" (");
            first = false;
        } else {
            printf(", ");
        }
        
        printf("%s=", prop->name);
        
        switch (prop->type) {
            case AST_PROP_STRING:
                printf("\"%s\"", prop->str_val ? prop->str_val : "");
                break;
            case AST_PROP_INT:
                printf("%lld", (long long)prop->int_val);
                break;
            case AST_PROP_FLOAT:
                printf("%g", prop->float_val);
                break;
            case AST_PROP_BOOL:
                printf("%s", prop->bool_val ? "true" : "false");
                break;
            default:
                printf("?");
                break;
        }
        
        prop = prop->next;
    }
    
    if (!first) printf(")");
    printf("\n");
    
    // Print children
    indent++;
    for (int i = 0; i < node->child_count; i++) {
        ast_print(node->children[i]);
    }
    indent--;
}
