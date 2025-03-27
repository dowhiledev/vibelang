#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>  // Add this for basename() function
#include "../utils/file_utils.h"
#include "../utils/ast.h"
#include "../utils/log_utils.h"
#include "../compiler/parser.h"
#include "../compiler/parser_utils.h"
#include "../utils/cache_utils.h"

void print_usage(const char* program_name) {
    fprintf(stderr, "Usage: %s [options] <input_file>\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -o <output_file>   Specify output file\n");
    fprintf(stderr, "  -d                 Enable debug mode\n");
    fprintf(stderr, "  -h                 Print this help message\n");
}

/**
 * External functions from other modules
 */
extern int generate_code(ast_node_t* ast, const char* output_file);
extern int cache_needs_update(const char* input_file, const char* output_file);

// Custom basename function to avoid platform-specific issues
char* get_basename(char* path) {
    char* last_slash = strrchr(path, '/');
    if (last_slash != NULL) {
        return last_slash + 1;
    }
    return path;
}

int main(int argc, char** argv) {
    char* input_file = NULL;
    char* output_file = NULL;
    int debug_mode = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-d") == 0) {
            debug_mode = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        } else {
            input_file = argv[i];
        }
    }
    
    // Set log level based on debug mode flag
    if (debug_mode) {
        set_log_level(LOG_LEVEL_DEBUG);
    } else {
        set_log_level(LOG_LEVEL_INFO);
    }
    
    // Check if input file is provided
    if (!input_file) {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }

    // Generate output filename if not specified
    if (output_file == NULL) {
        char* input_copy = strdup(input_file);
        char* base = get_basename(input_copy); // Use our custom function instead of basename
        
        // Create output file name with .c extension
        char* dot = strrchr(base, '.');
        if (dot != NULL) {
            *dot = '\0';  // Truncate at the extension
        }
        
        output_file = malloc(strlen(base) + 3); // +3 for .c and null terminator
        sprintf(output_file, "%s.c", base);
        free(input_copy);
    }
    
    INFO("Input file: %s", input_file);
    INFO("Output file: %s", output_file);

    // Check if we need to recompile based on timestamps
    if (!cache_needs_update(input_file, output_file)) {
        INFO("Output is up to date, skipping compilation");
        return 0;
    }

    // Read input file
    char* source = read_file(input_file);
    if (!source) {
        ERROR("Failed to read input file: %s", input_file);
        return 1;
    }

    // Create parser context
    vibe_context_t* ctx = vibe_create(source);
    if (!ctx) {
        ERROR("Failed to create parser context");
        free(source);
        return 1;
    }

    // Parse the code
    INFO("Parsing %s...", input_file);
    
    ast_node_t* ast = NULL;
    if (vibe_parse(ctx, &ast) && ast) {
        INFO("Parsing successful");

        // Debug: print AST if debug mode is on
        if (debug_mode) {
            ast_print(ast, 0);
        }

        // Generate code
        INFO("Generating code...");
        if (generate_code(ast, output_file)) {
            INFO("Code generation completed successfully");
        } else {
            ERROR("Code generation failed");
            vibe_destroy(ctx);
            free(source);
            if (ast) ast_node_free(ast);
            return 1;
        }
        
        // Clean up
        ast_node_free(ast);
    } else {
        ERROR("Parsing failed");
        vibe_destroy(ctx);
        free(source);
        return 1;
    }

    // Clean up
    vibe_destroy(ctx);
    free(source);
    
    // Free the output filename if it was dynamically allocated
    if (output_file != argv[2]) {
        free(output_file);
    }
    
    return 0;
}