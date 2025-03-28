#ifndef AST_H
#define AST_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>  // Added for size_t definition

// Limits for runtime checks
#define MAX_AST_DEPTH 1000
#define MAX_AST_NODES 1000000

// AST node types for the Vibe language
typedef enum ast_node_type_t {
    AST_PROGRAM,
    AST_FUNCTION_DECL,
    AST_FUNCTION_BODY,
    AST_TYPE_DECL,
    AST_VAR_DECL,
    AST_BASIC_TYPE,
    AST_MEANING_TYPE,
    AST_PARAM_LIST, 
    AST_PARAMETER,  
    AST_CLASS_DECL,
    AST_CLASS_BODY, 
    AST_MEMBER_VAR, 
    AST_IMPORT,
    AST_BLOCK,      
    AST_EXPR_STMT,  
    AST_RETURN_STMT,
    AST_PROMPT_BLOCK,
    AST_STRING_LITERAL,
    AST_INT_LITERAL,
    AST_FLOAT_LITERAL,
    AST_BOOL_LITERAL,
    AST_IDENTIFIER,
    AST_CALL_EXPR    
} ast_node_type_t;

// Forward declaration of AST node structure
typedef struct ast_node_t ast_node_t;

// Helper list structure for grammatical constructs that return lists
typedef struct ast_list_t {
    ast_node_t** list;
    size_t len;
} ast_list_t;

// Helper array functions to work with the grammar
#define pcc_array_length(a) ((a) ? (a)->len : 0)
#define pcc_array_get(a, i) ((i) < pcc_array_length(a) ? (a)->list[i] : NULL)

// Property types for AST nodes
typedef enum {
    AST_PROP_NONE,
    AST_PROP_STRING,
    AST_PROP_INT,
    AST_PROP_FLOAT,
    AST_PROP_BOOL
} ast_prop_type_t;

// Property structure for AST nodes
typedef struct ast_prop_t {
    char* name;
    ast_prop_type_t type;
    union {
        char* str_val;
        int64_t int_val;
        double float_val;
        bool bool_val;
    };
    struct ast_prop_t* next;
} ast_prop_t;

// AST node structure - the basic building block of our syntax tree
struct ast_node_t {
    ast_node_type_t type;
    
    // Properties (name-value pairs)
    ast_prop_t* properties;
    
    // Child nodes
    ast_node_t** children;
    int child_count;
    int child_capacity;
    
    // Source location
    int line;
    int column;
    
    // Parent node (for traversal)
    ast_node_t* parent;
};

// Functions for creating and managing AST nodes
ast_node_t* create_ast_node(ast_node_type_t type);
void ast_node_free(ast_node_t* node);

// Functions for managing children
void ast_add_child(ast_node_t* parent, ast_node_t* child);
void ast_remove_child(ast_node_t* parent, int index);
void ast_replace_child(ast_node_t* parent, int index, ast_node_t* new_child);

// Functions for managing properties
void ast_set_string(ast_node_t* node, const char* name, const char* value);
void ast_set_int(ast_node_t* node, const char* name, int64_t value);
void ast_set_float(ast_node_t* node, const char* name, double value);
void ast_set_bool(ast_node_t* node, const char* name, bool value);

const char* ast_get_string(const ast_node_t* node, const char* name);
int64_t ast_get_int(const ast_node_t* node, const char* name);
double ast_get_float(const ast_node_t* node, const char* name);
bool ast_get_bool(const ast_node_t* node, const char* name);

// Debug and printing functions
void ast_print(const ast_node_t* node);
void ast_reset_metrics();
void ast_get_metrics(int* depth, int* count);
const char* ast_node_type_name(ast_node_type_t type);

#endif /* AST_H */
