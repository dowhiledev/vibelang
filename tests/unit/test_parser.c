#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/utils/ast.h"
#include "../../src/compiler/parser.h"
#include "../../src/compiler/parser_utils.h"
#include "../../src/utils/log_utils.h"

// Test parsing a simple function declaration
static void test_function_declaration() {
    const char* source = 
        "fn hello(name: String) -> String {\n"
        "    return \"Hello, \" + name;\n"
        "}\n";
    
    // Use the const void* signature now - no cast needed
    vibe_context_t* ctx = vibe_create(source);
    assert(ctx != NULL && "Failed to create parser context");
    
    ast_node_t* ast = NULL;
    int result = vibe_parse(ctx, &ast);
    
    assert(result == 1 && "Parsing should succeed");
    assert(ast != NULL && "AST should not be NULL");
    assert(ast->type == AST_PROGRAM && "Root node should be a program");
    
    // Validate the program has one child which is a function
    assert(ast->child_count == 1 && "Program should have one child");
    assert(ast->children[0]->type == AST_FUNCTION_DECL && "Child should be a function declaration");
    
    // Validate function name
    const char* func_name = ast_get_string(ast->children[0], "name");
    assert(strcmp(func_name, "hello") == 0 && "Function name should be 'hello'");
    
    // Clean up
    ast_node_free(ast);
    vibe_destroy(ctx);
    
    printf("✅ test_function_declaration passed\n");
}

// Test parsing a type declaration
static void test_type_declaration() {
    const char* source = 
        "type Temperature = Meaning<Int>(\"temperature in Celsius\");\n";
    
    // Use the const void* signature now - no cast needed
    vibe_context_t* ctx = vibe_create(source);
    assert(ctx != NULL && "Failed to create parser context");
    
    ast_node_t* ast = NULL;
    int result = vibe_parse(ctx, &ast);
    
    assert(result == 1 && "Parsing should succeed");
    assert(ast != NULL && "AST should not be NULL");
    assert(ast->type == AST_PROGRAM && "Root node should be a program");
    
    // Validate the program has one child which is a type declaration
    assert(ast->child_count == 1 && "Program should have one child");
    assert(ast->children[0]->type == AST_TYPE_DECL && "Child should be a type declaration");
    
    // Validate type name
    const char* type_name = ast_get_string(ast->children[0], "name");
    assert(strcmp(type_name, "Temperature") == 0 && "Type name should be 'Temperature'");
    
    // Clean up
    ast_node_free(ast);
    vibe_destroy(ctx);
    
    printf("✅ test_type_declaration passed\n");
}

// Test parsing a class declaration
static void test_class_declaration() {
    const char* source = 
        "class Person {\n"
        "    name: String;\n"
        "    age: Int;\n"
        "    fn greet() -> String {\n"
        "        return \"Hello, I am \" + name;\n"
        "    }\n"
        "}\n";
    
    // Use the const void* signature now - no cast needed
    vibe_context_t* ctx = vibe_create(source);
    assert(ctx != NULL && "Failed to create parser context");
    
    ast_node_t* ast = NULL;
    int result = vibe_parse(ctx, &ast);
    
    assert(result == 1 && "Parsing should succeed");
    assert(ast != NULL && "AST should not be NULL");
    assert(ast->type == AST_PROGRAM && "Root node should be a program");
    
    // Validate the program has one child which is a class declaration
    assert(ast->child_count == 1 && "Program should have one child");
    assert(ast->children[0]->type == AST_CLASS_DECL && "Child should be a class declaration");
    
    // Validate class name
    const char* class_name = ast_get_string(ast->children[0], "name");
    assert(strcmp(class_name, "Person") == 0 && "Class name should be 'Person'");
    
    // Validate class has 3 children (2 member vars and 1 method)
    assert(ast->children[0]->child_count == 3 && "Class should have 3 members");
    
    // Clean up
    ast_node_free(ast);
    vibe_destroy(ctx);
    
    printf("✅ test_class_declaration passed\n");
}

// Test parsing a function with a prompt statement
static void test_prompt_statement() {
    const char* source = 
        "fn getWeather(city: String) -> String {\n"
        "    prompt \"What is the weather like in {city}?\";\n"
        "}\n";
    
    // Use the const void* signature now - no cast needed
    vibe_context_t* ctx = vibe_create(source);
    assert(ctx != NULL && "Failed to create parser context");
    
    ast_node_t* ast = NULL;
    int result = vibe_parse(ctx, &ast);
    
    assert(result == 1 && "Parsing should succeed");
    assert(ast != NULL && "AST should not be NULL");
    
    // Validate function has a body with a prompt statement
    ast_node_t* func = ast->children[0];
    assert(func->child_count >= 1 && "Function should have a body");
    
    // The function body is the last child
    ast_node_t* body = NULL;
    for (int i = 0; i < func->child_count; i++) {
        if (func->children[i]->type == AST_FUNCTION_BODY) {
            body = func->children[i];
            break;
        }
    }
    
    assert(body != NULL && "Function body should exist");
    assert(body->child_count == 1 && "Function body should have 1 statement");
    assert(body->children[0]->type == AST_PROMPT_BLOCK && "Statement should be a prompt");
    
    // Clean up
    ast_node_free(ast);
    vibe_destroy(ctx);
    
    printf("✅ test_prompt_statement passed\n");
}

int main() {
    printf("Running parser tests...\n");
    
    test_function_declaration();
    test_type_declaration();
    test_class_declaration();
    test_prompt_statement();
    
    printf("All parser tests passed! 🎉\n");
    return 0;
}
