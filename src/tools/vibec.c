#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/ast.h"
#include "../utils/file_utils.h"
#include "../utils/log_utils.h"
#include "../compiler/parser_utils.h"  // Changed from "../compiler/parser.h" to use parser_utils.h
#include "../compiler/semantic.h"
#include "../compiler/codegen.h"

#define VERSION "0.1.0"

// Function to display help information
void show_help() {
    printf("VibeC Compiler %s\n", VERSION);
    printf("Usage: vibec [options] <input_file>\n\n");
    printf("Options:\n");
    printf("  -h, --help          Show this help message\n");
    printf("  -o, --output FILE   Specify output file name\n");
    printf("  -v, --verbose       Enable verbose output\n");
    printf("  -d, --debug         Enable debug mode\n");
    printf("  -c, --check         Only check syntax and semantics, don't generate code\n");
    printf("  --version           Show version information\n");
}

// Function to display version information
void show_version() {
    printf("VibeC Compiler %s\n", VERSION);
    printf("Copyright (C) 2023 Vibe Language Team.\n");
}

int main(int argc, char** argv) {
    // Default values
    char* input_file = NULL;
    char* output_file = NULL;
    int verbose = 0;
    int debug = 0;
    int check_only = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_help();
            return 0;
        } 
        else if (strcmp(argv[i], "--version") == 0) {
            show_version();
            return 0;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug = 1;
        }
        else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--check") == 0) {
            check_only = 1;
        }
        else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                fprintf(stderr, "Error: Missing output file name after %s\n", argv[i]);
                return 1;
            }
        }
        else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            return 1;
        }
        else {
            if (input_file == NULL) {
                input_file = argv[i];
            } else {
                fprintf(stderr, "Error: Multiple input files not supported\n");
                return 1;
            }
        }
    }
    
    // Check if input file is specified
    if (input_file == NULL) {
        fprintf(stderr, "Error: No input file specified\n");
        show_help();
        return 1;
    }
    
    // Initialize logging
    log_level_t log_level = LOG_LEVEL_INFO;
    if (debug) {
        log_level = LOG_LEVEL_DEBUG;
    } else if (verbose) {
        log_level = LOG_LEVEL_INFO;
    } else {
        log_level = LOG_LEVEL_WARN;
    }
    
    init_logging(log_level);
    
    // Read the input file
    INFO("Reading input file: %s", input_file);
    char* source_code = read_file(input_file);
    if (source_code == NULL) {
        ERROR("Failed to read input file: %s", input_file);
        return 1;
    }
    
    // Parse the source code
    INFO("Parsing source code...");
    ast_node_t* ast = parse_string(source_code);
    free(source_code); // Done with the source code
    
    if (ast == NULL) {
        ERROR("Failed to parse input file");
        return 1;
    }
    
    DEBUG("AST created successfully");
    
    // Perform semantic analysis
    INFO("Running semantic analysis...");
    int semantic_result = analyze_semantics(ast);
    if (semantic_result != 0) {
        ERROR("Semantic analysis failed");
        ast_node_free(ast);
        return 1;
    }
    
    INFO("Semantic analysis completed successfully");
    
    // Generate code if not in check-only mode
    if (!check_only) {
        // Determine output file name if not specified
        if (output_file == NULL) {
            // Remove .vibe extension if present and add .c
            output_file = malloc(strlen(input_file) + 3); // +3 for .c and null terminator
            if (output_file == NULL) {
                ERROR("Memory allocation failed");
                ast_node_free(ast);
                return 1;
            }
            
            strcpy(output_file, input_file);
            char* dot = strrchr(output_file, '.');
            if (dot && strcmp(dot, ".vibe") == 0) {
                *dot = '\0';
            }
            strcat(output_file, ".c");
        }
        
        INFO("Generating code to: %s", output_file);
        int codegen_result = generate_code(ast, output_file);
        
        if (codegen_result != 0) {
            ERROR("Code generation failed");
            ast_node_free(ast);
            if (output_file != NULL && output_file != argv[argc - 1]) {
                free(output_file);
            }
            return 1;
        }
        
        INFO("Code generation completed successfully");
    }
    
    // Clean up
    ast_node_free(ast);
    if (output_file != NULL && output_file != argv[argc - 1]) {
        free(output_file);
    }
    
    INFO("Compilation completed successfully");
    return 0;
}