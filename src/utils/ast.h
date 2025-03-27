#ifndef VIBELANG_AST_H
#define VIBELANG_AST_H

#include <stddef.h>

/* AST Node Types */
typedef enum {
    AST_PROGRAM,
    AST_IMPORT,
    AST_TYPE_DECL,
    AST_MEANING_TYPE,
    AST_BASIC_TYPE,
    AST_FUNCTION_DECL,
    AST_PARAM_LIST,
    AST_PARAMETER,
    AST_FUNCTION_BODY,
    AST_PROMPT_BLOCK,
    AST_CLASS_DECL,
    AST_MEMBER_VAR,
    AST_VAR_DECL,
    AST_EXPR_STMT,
    AST_RETURN_STMT,
    AST_BLOCK,
    AST_IDENTIFIER,
    AST_INT_LITERAL,
    AST_FLOAT_LITERAL,
    AST_STRING_LITERAL,
    AST_BOOL_LITERAL,
    AST_CALL_EXPR
} ast_node_type_t;

/* AST Node Value Types */
typedef enum {
    AST_VAL_NONE,
    AST_VAL_INT,
    AST_VAL_FLOAT,
    AST_VAL_STRING,
    AST_VAL_BOOL
} ast_val_type_t;

/* AST Node Value Union */
typedef union {
    long long int_val;
    double float_val;
    char* str_val;
    int bool_val;
} ast_val_t;

/* AST Node */
typedef struct ast_node {
    ast_node_type_t type;
    
    /* Properties storage */
    struct {
        char* key;
        ast_val_type_t type;
        ast_val_t val;
    } *props;
    size_t prop_count;
    size_t prop_capacity;
    
    /* Children nodes */
    struct ast_node** children;
    size_t child_count;
    size_t child_capacity;
    
    /* Source location info */
    int line;
    int column;
} ast_node_t;

/* AST List type used in parser */
typedef struct {
    ast_node_t** list;
    size_t len;
} ast_list_t;

/* AST Creation and Management */
ast_node_t* create_ast_node(ast_node_type_t type);
void ast_node_free(ast_node_t* node);
void ast_add_child(ast_node_t* parent, ast_node_t* child);

/* AST Node Property Functions */
void ast_set_int(ast_node_t* node, const char* key, long long value);
void ast_set_float(ast_node_t* node, const char* key, double value);
void ast_set_string(ast_node_t* node, const char* key, const char* value);
void ast_set_bool(ast_node_t* node, const char* key, int value);

long long ast_get_int(const ast_node_t* node, const char* key);
double ast_get_float(const ast_node_t* node, const char* key);
const char* ast_get_string(const ast_node_t* node, const char* key);
int ast_get_bool(const ast_node_t* node, const char* key);

/* AST Debug Functions */
void ast_print(const ast_node_t* node, int depth);  // Add this declaration
size_t ast_child_count(const ast_node_t* node);     // Utility function
ast_node_t* ast_child_get(const ast_node_t* node, size_t index);  // Utility function

#endif /* VIBELANG_AST_H */
