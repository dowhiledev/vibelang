#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/utils/ast.h"
#include "../../src/utils/file_utils.h"
#include "../../include/symbol_table.h"

// External functions that we'll test
extern int generate_code(ast_node_t* ast, const char* output_file);
extern ast_node_t* parse_string(const char* source);

// Utility to compare files
int files_are_equal(const char* file1, const char* file2) {
    FILE *f1, *f2;
    int c1, c2;
    
    f1 = fopen(file1, "r");
    if (!f1) return 0;
    
    f2 = fopen(file2, "r");
    if (!f2) {
        fclose(f1);
        return 0;
    }
    
    do {
        c1 = getc(f1);
        c2 = getc(f2);
    } while (c1 != EOF && c2 != EOF && c1 == c2);
    
    fclose(f1);
    fclose(f2);
    
    return c1 == c2;
}

// Helper to create a test case from VibeLang source
void test_codegen(const char* name, const char* source) {
    char source_path[256];
    char expected_path[256];
    char output_path[256];
    
    // Set up paths
    sprintf(source_path, "tests/unit/data/%s.vibe", name);
    sprintf(expected_path, "tests/unit/data/%s.expected.c", name);
    sprintf(output_path, "tests/unit/data/%s.output.c", name);
    
    // Create source file
    FILE* src_file = fopen(source_path, "w");
    assert(src_file);
    fputs(source, src_file);
    fclose(src_file);
    
    // Parse the source
    char* src_content = read_file(source_path);
    assert(src_content);
    
    ast_node_t* ast = parse_string(src_content);
    assert(ast);
    
    free(src_content);
    
    // Generate code
    int result = generate_code(ast, output_path);
    assert(result);
    
    // Compare with expected output
    if (file_exists(expected_path)) {
        assert(files_are_equal(output_path, expected_path));
        printf("Test %s passed: generated code matches expected\n", name);
    } else {
        printf("Test %s: no expected file, generated output saved to %s\n", name, output_path);
    }
    
    // Clean up
    ast_node_free(ast);
}

// Test simple function with prompt
void test_simple_function() {
    const char* source =
        "type Temperature = Meaning<Int>(\"temperature in Celsius\");\n"
        "\n"
        "fn getTemperature(city: String) -> Temperature {\n"
        "    prompt \"What is the temperature in {city}?\"\n"
        "}\n";
    
    test_codegen("simple_function", source);
}

// Test function with variables
void test_function_with_vars() {
    const char* source =
        "type Weather = Meaning<String>(\"weather description\");\n"
        "\n"
        "fn getWeather(city: String, day: String) -> Weather {\n"
        "    let location = city;\n"
        "    let when = day;\n"
        "    prompt \"What is the weather like in {location} on {when}?\"\n"
        "}\n";
    
    test_codegen("function_with_vars", source);
}

// Main test runner
int main() {
    printf("Running code generator tests...\n");
    
    test_simple_function();
    test_function_with_vars();
    
    printf("All code generator tests passed!\n");
    return 0;
}
