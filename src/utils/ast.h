#ifndef AST_H
#define AST_H

#include <stddef.h>

// Maximum depth of AST nesting to prevent stack overflow
#ifndef MAX_AST_DEPTH 
#define MAX_AST_DEPTH 100
#endif

// Maximum number of AST nodes to prevent memory exhaustion
#ifndef MAX_AST_NODES
#define MAX_AST_NODES 10000
#endif

// Forward declaration for ast types
typedef enum ast_node_type_tag ast_node_type_t;
typedef struct ast_node_tag ast_node_t;
typedef struct ast_list_tag ast_list_t;

// Reset AST metrics at the beginning of parsing
void ast_reset_metrics();

// Get current AST metrics for diagnostics
void ast_get_metrics(int* depth, int* count);

/**
 * Print AST node structure with indentation
 * @param node The AST node to print
 * @param indent The current indentation level
 */
void ast_print(const ast_node_t* node, int indent);

// Enumeration of AST node types
enum ast_node_type_tag {
    AST_PROGRAM,
    AST_FUNCTION_DECL,
    AST_FUNCTION_BODY,
    AST_PARAM_LIST,
    AST_PARAMETER,
    AST_TYPE_DECL,
    AST_BASIC_TYPE,
    AST_MEANING_TYPE,
    AST_CLASS_DECL,
    AST_MEMBER_VAR,
    AST_IMPORT,
    AST_BLOCK,
    AST_VAR_DECL,
    AST_RETURN_STMT,
    AST_PROMPT_BLOCK,
    AST_EXPR_STMT,
    AST_CALL_EXPR,
    AST_STRING_LITERAL,
    AST_INT_LITERAL,
    AST_FLOAT_LITERAL,
    AST_BOOL_LITERAL,
    AST_IDENTIFIER
};

// Enumeration of AST property value types
typedef enum {
    AST_VAL_INT,
    AST_VAL_FLOAT,
    AST_VAL_BOOL,
    AST_VAL_STRING,
    AST_VAL_POINTER
} ast_value_type_t;

// Union for AST property values
typedef union {
    long long int_val;     // Changed from int to long long
    double float_val;      // Changed from float to double
    int bool_val;
    char* str_val;
    void* ptr_val;
} ast_value_t;

// Structure for AST property
typedef struct {
    char* key;
    ast_value_type_t type;
    ast_value_t val;
} ast_prop_t;

// Structure for AST node
struct ast_node_tag {
    ast_node_type_t type;
    
    // Node properties (key-value pairs)
    ast_prop_t* props;
    size_t prop_count;
    size_t prop_capacity;
    
    // Child nodes
    ast_node_t** children;
    size_t child_count;
    size_t child_capacity;
    
    // Source location
    int line;
    int column;
};

// Structure for AST list (used by the parser)
struct ast_list_tag {
    ast_node_t** list;
    size_t len;
};

// AST node creation and management
ast_node_t* create_ast_node(ast_node_type_t type);
void ast_node_free(ast_node_t* node);
void ast_add_child(ast_node_t* parent, ast_node_t* child);

// AST property management - update parameter types to match implementation
void ast_set_int(ast_node_t* node, const char* key, long long value);
void ast_set_float(ast_node_t* node, const char* key, double value);
void ast_set_bool(ast_node_t* node, const char* key, int value);
void ast_set_string(ast_node_t* node, const char* key, const char* value);
void ast_set_pointer(ast_node_t* node, const char* key, void* value);

// AST property access - update return and parameter types to match implementation
long long ast_get_int(const ast_node_t* node, const char* key);
double ast_get_float(const ast_node_t* node, const char* key);
int ast_get_bool(const ast_node_t* node, const char* key);
const char* ast_get_string(const ast_node_t* node, const char* key);
void* ast_get_pointer(const ast_node_t* node, const char* key);

// AST serialization 
char* ast_to_json(ast_node_t* node);

#endif /* AST_H */
