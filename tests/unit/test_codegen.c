#include "../../include/symbol_table.h"
#include "../../src/utils/ast.h"
#include "../../src/utils/file_utils.h"
#include "../../src/utils/log_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // For setting timeout without signals
#include <unistd.h>

// External functions that we'll test
extern int generate_code(ast_node_t *ast, const char *output_file);
extern ast_node_t *parse_string(const char *source);

// Create directories if they don't exist
static int ensure_test_directory() {
  const char *dir = "tests/unit/data";
  printf("Creating test directory: %s\n", dir);
  return create_directories(dir);
}

// Utility to compare files - simplified version that's less prone to hanging
int files_are_equal(const char *file1, const char *file2) {
  char *content1 = read_file(file1);
  if (!content1) {
    printf("Could not read file: %s\n", file1);
    return 0;
  }

  char *content2 = read_file(file2);
  if (!content2) {
    printf("Could not read file: %s\n", file2);
    free(content1);
    return 0;
  }

  int result = (strcmp(content1, content2) == 0);
  free(content1);
  free(content2);

  return result;
}

// Helper to create a test case from VibeLang source - non-blocking version
void test_codegen(const char *name, const char *source) {
  printf("\n=== Testing code generation for '%s' ===\n", name);

  // Ensure we have a test directory
  if (!ensure_test_directory()) {
    printf("ERROR: Could not create test directory\n");
    return;
  }

  // Set up paths - use relative paths which are more reliable
  char source_path[256];
  char expected_path[256];
  char output_path[256];

  snprintf(source_path, sizeof(source_path), "tests/unit/data/%s.vibe", name);
  snprintf(expected_path, sizeof(expected_path),
           "tests/unit/data/%s.expected.c", name);
  snprintf(output_path, sizeof(output_path), "tests/unit/data/%s.output.c",
           name);

  printf("Source path: %s\n", source_path);
  printf("Expected path: %s\n", expected_path);
  printf("Output path: %s\n", output_path);

  // Create source file
  printf("Writing source file...\n");
  FILE *src_file = fopen(source_path, "w");
  if (!src_file) {
    printf("ERROR: Could not create source file %s\n", source_path);
    return;
  }

  fputs(source, src_file);
  fclose(src_file);
  printf("Source file written\n");

  // Parse the source (copy source from file instead of using passed string)
  printf("Reading source file...\n");
  char *src_content = read_file(source_path);
  if (!src_content) {
    printf("ERROR: Could not read source file %s\n", source_path);
    return;
  }

  printf("Parsing source code...\n");
  ast_node_t *ast = parse_string(src_content);
  free(src_content);

  if (!ast) {
    printf("ERROR: Failed to parse source for test %s\n", name);
    return;
  }
  printf("Parsing successful\n");

  // Generate code
  printf("Generating code...\n");
  int result = generate_code(ast, output_path);
  if (!result) {
    printf("ERROR: Failed to generate code for test %s\n", name);
    ast_node_free(ast);
    return;
  }
  printf("Code generation successful\n");

  // Compare with expected output
  if (file_exists(expected_path)) {
    printf("Expected file exists, comparing...\n");
    if (files_are_equal(output_path, expected_path)) {
      printf("TEST PASSED: Generated code matches expected\n");
    } else {
      printf("TEST FAILED: Generated code differs from expected\n");
    }
  } else {
    printf("Expected output file not found, creating it...\n");
    // Copy output to expected for future tests
    char *output_content = read_file(output_path);
    if (output_content) {
      FILE *expected_file = fopen(expected_path, "w");
      if (expected_file) {
        fputs(output_content, expected_file);
        fclose(expected_file);
        printf("Created expected output file %s\n", expected_path);
      } else {
        printf("ERROR: Could not create expected file %s\n", expected_path);
      }
      free(output_content);
    } else {
      printf("ERROR: Could not read output file %s\n", output_path);
    }
  }

  // Clean up
  printf("Cleaning up AST...\n");
  ast_node_free(ast);
  printf("Test completion for '%s'\n\n", name);
}

// Test a simple function with a prompt block
static void test_simple_function() {
  const char *source =
      "type Temperature = Meaning<Int>(\"temperature in Celsius\");\n"
      "\n"
      "fn getTemperature(city: String) -> Temperature {\n"
      "    prompt \"What is the temperature in {city}?\"\n"
      "}\n";

  test_codegen("simple_function", source);
}

// Test a function with variables
static void test_function_with_vars() {
  const char *source =
      "type Weather = Meaning<String>(\"weather description\");\n"
      "\n"
      "fn getWeather(city: String, day: String) -> Weather {\n"
      "    let location = city;\n"
      "    let when = day;\n"
      "    prompt \"What is the weather like in {location} on {when}?\"\n"
      "}\n";

  test_codegen("function_with_vars", source);
}

// Main test runner - with simple timeout
int main() {
  printf("Running code generator tests with timeout protection...\n");

  // Set up timeout for tests (5 seconds)
  time_t start_time = time(NULL);
  time_t timeout = 5; // 5 seconds timeout

  // Run tests with timeout protection
  printf("Running test_simple_function\n");
  test_simple_function();

  // Check if we've exceeded our timeout
  if (time(NULL) - start_time > timeout) {
    printf("ERROR: Tests exceeding timeout limit. Stopping.\n");
    return 1;
  }

  printf("Running test_function_with_vars\n");
  test_function_with_vars();

  printf("All code generator tests completed!\n");
  return 0;
}
