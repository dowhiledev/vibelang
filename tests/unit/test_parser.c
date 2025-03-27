#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/utils/ast.h"
#include "../../src/compiler/parser.h"

// Test parsing function prototype
static void test_function_parsing() {
    const char* source = 
        "fn testFunction(x: Int, y: String) -> Int {\n"
        "    let z = x;\n"
        "    return z;\n"
        "}\n";
    
    vibe_context_t* ctx = vibe_create(source);
    assert(ctx != NULL);
    
    ast_node_t* ast = NULL;
    int result = vibe_parse(ctx, &ast);
    
    assert(result != 0);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->child_count == 1);
    
    // Check function declaration
    ast_node_t* func = ast->children[0];
    assert(func->type == AST_FUNCTION_DECL);
    assert(strcmp(ast_get_string(func, "name"), "testFunction") == 0);
    
    // Check return type
    int found_return_type = 0;
    for (size_t i = 0; i < func->child_count; i++) {
        if (func->children[i]->type == AST_BASIC_TYPE) {
            assert(strcmp(ast_get_string(func->children[i], "type"), "Int") == 0);
            found_return_type = 1;
            break;
        }
    }
    assert(found_return_type);
    
    // Find param list
    ast_node_t* params = NULL;
    for (size_t i = 0; i < func->child_count; i++) {
        if (func->children[i]->type == AST_PARAM_LIST) {
            params = func->children[i];
            break;
        }
    }
    assert(params != NULL);
    assert(params->child_count == 2);
    
    // Check first parameter
    ast_node_t* param1 = params->children[0];
    assert(param1->type == AST_PARAMETER);
    assert(strcmp(ast_get_string(param1, "name"), "x") == 0);
    assert(param1->child_count == 1);
    assert(param1->children[0]->type == AST_BASIC_TYPE);
    assert(strcmp(ast_get_string(param1->children[0], "type"), "Int") == 0);
    
    // Check second parameter
    ast_node_t* param2 = params->children[1];
    assert(param2->type == AST_PARAMETER);
    assert(strcmp(ast_get_string(param2, "name"), "y") == 0);
    assert(param2->child_count == 1);
    assert(param2->children[0]->type == AST_BASIC_TYPE);
    assert(strcmp(ast_get_string(param2->children[0], "type"), "String") == 0);
    
    // Clean up
    vibe_destroy(ctx);
    ast_node_free(ast);
    
    printf("Function parsing test passed\n");
}

// Test type definition parsing
static void test_type_parsing() {
    const char* source = 
        "type Temperature = Meaning<Int>(\"temperature in Celsius\");\n"
        "type Name = String;\n";
    
    vibe_context_t* ctx = vibe_create(source);
    assert(ctx != NULL);
    
    ast_node_t* ast = NULL;
    int result = vibe_parse(ctx, &ast);
    
    assert(result != 0);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->child_count == 2);
    
    // Check first type declaration (Meaning type)
    ast_node_t* type1 = ast->children[0];
    assert(type1->type == AST_TYPE_DECL);
    assert(strcmp(ast_get_string(type1, "name"), "Temperature") == 0);
    assert(type1->child_count == 1);
    assert(type1->children[0]->type == AST_MEANING_TYPE);
    assert(strcmp(ast_get_string(type1->children[0], "meaning"), "temperature in Celsius") == 0);
    
    // Check second type declaration (Basic type alias)
    ast_node_t* type2 = ast->children[1];
    assert(type2->type == AST_TYPE_DECL);
    assert(strcmp(ast_get_string(type2, "name"), "Name") == 0);
    assert(type2->child_count == 1);
    assert(type2->children[0]->type == AST_BASIC_TYPE);
    assert(strcmp(ast_get_string(type2->children[0], "type"), "String") == 0);
    
    // Clean up
    vibe_destroy(ctx);
    ast_node_free(ast);
    
    printf("Type parsing test passed\n");
}

// Test prompt block parsing
static void test_prompt_parsing() {
    const char* source = 
        "fn getWeather(city: String) -> String {\n"
        "    prompt \"What is the weather like in {city}?\"\n"
        "}\n";
    
    vibe_context_t* ctx = vibe_create(source);
    assert(ctx != NULL);
    
    ast_node_t* ast = NULL;
    int result = vibe_parse(ctx, &ast);
    
    assert(result != 0);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->child_count == 1);
    
    // Find function body
    ast_node_t* func = ast->children[0];
    ast_node_t* body = NULL;
    for (size_t i = 0; i < func->child_count; i++) {
        if (func->children[i]->type == AST_FUNCTION_BODY) {
            body = func->children[i];
            break;
        }
    }
    assert(body != NULL);
    assert(body->child_count == 1);
    
    // Check prompt block
    ast_node_t* prompt = body->children[0];
    assert(prompt->type == AST_PROMPT_BLOCK);
    assert(strcmp(ast_get_string(prompt, "template"), "What is the weather like in {city}?") == 0);
    
    // Clean up
    vibe_destroy(ctx);
    ast_node_free(ast);
    
    printf("Prompt parsing test passed\n");
}

// Test class declaration parsing
static void test_class_parsing() {
    const char* source = 
        "class Person {\n"
        "    name: String;\n"
        "    age: Int;\n"
        "    fn greet() -> String {\n"
        "        return \"Hello\";\n"
        "    }\n"
        "}\n";
    
    vibe_context_t* ctx = vibe_create(source);
    assert(ctx != NULL);
    
    ast_node_t* ast = NULL;
    int result = vibe_parse(ctx, &ast);
    
    assert(result != 0);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->child_count == 1);
    
    // Check class declaration
    ast_node_t* class = ast->children[0];
    assert(class->type == AST_CLASS_DECL);
    assert(strcmp(ast_get_string(class, "name"), "Person") == 0);
    assert(class->child_count == 3);
    
    // Check member variables
    int found_name = 0;
    int found_age = 0;
    int found_method = 0;
    
    for (size_t i = 0; i < class->child_count; i++) {
        ast_node_t* child = class->children[i];
        
        if (child->type == AST_MEMBER_VAR) {
            if (strcmp(ast_get_string(child, "name"), "name") == 0) {
                found_name = 1;
                assert(child->child_count == 1);
                assert(child->children[0]->type == AST_BASIC_TYPE);
                assert(strcmp(ast_get_string(child->children[0], "type"), "String") == 0);
            } else if (strcmp(ast_get_string(child, "name"), "age") == 0) {
                found_age = 1;
                assert(child->child_count == 1);
                assert(child->children[0]->type == AST_BASIC_TYPE);
                assert(strcmp(ast_get_string(child->children[0], "type"), "Int") == 0);
            }
        } else if (child->type == AST_FUNCTION_DECL) {
            found_method = 1;
            assert(strcmp(ast_get_string(child, "name"), "greet") == 0);
        }
    }
    
    assert(found_name);
    assert(found_age);
    assert(found_method);
    
    // Clean up
    vibe_destroy(ctx);
    ast_node_free(ast);
    
    printf("Class parsing test passed\n");
}

int main() {
    printf("Running parser tests...\n");
    
    test_function_parsing();
    test_type_parsing();
    test_prompt_parsing();
    test_class_parsing();
    
    printf("All parser tests passed!\n");
    return 0;
}
