#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/utils/ast.h"
#include "../../src/compiler/parser_bison.h"
#include "../../src/utils/log_utils.h"

// Test cases for the parser
static void test_simple_function() {
    const char* source = 
        "fn test() { return; }";
    
    printf("Parsing: %s\n", source);
    ast_node_t* ast = parse_string(source);
    
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->child_count > 0);
    
    // First child should be a function declaration
    ast_node_t* func = ast->children[0];
    assert(func->type == AST_FUNCTION_DECL);
    assert(strcmp(ast_get_string(func, "name"), "test") == 0);
    
    // Cleanup
    ast_node_free(ast);
    printf("Simple function test passed!\n");
}

static void test_type_declaration() {
    const char* source = 
        "type Temperature = Meaning<Int>(\"temperature in Celsius\");";
    
    printf("Parsing: %s\n", source);
    ast_node_t* ast = parse_string(source);
    
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->child_count > 0);
    
    // First child should be a type declaration
    ast_node_t* type_decl = ast->children[0];
    assert(type_decl->type == AST_TYPE_DECL);
    assert(strcmp(ast_get_string(type_decl, "name"), "Temperature") == 0);
    
    // Cleanup
    ast_node_free(ast);
    printf("Type declaration test passed!\n");
}

int main() {
    // Initialize logging
    init_logging(LOG_LEVEL_DEBUG);
    
    printf("Running Bison parser tests...\n");
    
    // Run tests
    test_simple_function();
    test_type_declaration();
    
    printf("All Bison parser tests passed!\n");
    return 0;
}
