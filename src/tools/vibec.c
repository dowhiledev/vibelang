#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "../utils/ast.h"
#include "../utils/log_utils.h"
#include "../utils/file_utils.h"

/* Include the parser header if available */
#if __has_include("../compiler/parser.h")
#include "../compiler/parser.h"
#else
/* Forward declare parser functions if header not available */
typedef struct vibe_context_t vibe_context_t;
vibe_context_t* vibe_create(const char* input);
int vibe_parse(vibe_context_t* ctx, ast_node_t** ast);
const char* vibe_get_error(vibe_context_t* ctx);
void vibe_destroy(vibe_context_t* ctx);
#endif

// Forward declarations
extern int semantic_analyze(ast_node_t* ast);
extern void semantic_cleanup(void);
extern int generate_code(ast_node_t* ast, const char* output_file);
extern int cache_needs_update(const char* input_file, const char* output_file);

void print_usage(const char* program_name) {
    printf("VibeLang Compiler\n");
    printf("Usage: %s [options] input_file\n", program_name);
    printf("\nOptions:\n");
    printf("  -o <file>   Specify output file\n");
    printf("  -c          Compile only, don't link\n");
    printf("  -v          Verbose output\n");
    printf("  -h          Display this help message\n");
}

int main(int argc, char *argv[]) {
    char* input_file = NULL;
    char* output_file = NULL;
    int compile_only = 0;
    int verbose = 0;
    
    // Initialize logging
    log_init(LOG_LEVEL_INFO);
    
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0) {
            compile_only = 1;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
            log_set_level(LOG_LEVEL_DEBUG);
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        } else {
            if (input_file != NULL) {
                fprintf(stderr, "Multiple input files specified\n");
                print_usage(argv[0]);
                return 1;
            }
            input_file = argv[i];
        }
    }
    
    if (input_file == NULL) {
        fprintf(stderr, "No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // If no output file specified, use input file name with .c extension
    if (output_file == NULL) {
        char* input_copy = strdup(input_file);
        char* base = basename(input_copy);
        
        // Replace or append .c extension
        char* dot = strrchr(base, '.');
        if (dot != NULL) {
            *dot = '\0';
        }
        
        output_file = malloc(strlen(base) + 3);
        sprintf(output_file, "%s.c", base);
        free(input_copy);
    }
    
    if (verbose) {
        INFO("Input file: %s", input_file);
        INFO("Output file: %s", output_file);
    }
    
    // Check if the cache is up-to-date
    if (!compile_only && cache_needs_update(input_file, output_file)) {
        INFO("Output is up to date, skipping compilation");
        free(output_file);
        return 0;
    }
    
    // Read the input file
    char* source = read_file(input_file);
    if (!source) {
        ERROR("Failed to read input file: %s", input_file);
        free(output_file);
        return 1;
    }
    
    // Parse the source code
    INFO("Parsing %s...", input_file);
    vibe_context_t* ctx = vibe_create(source);
    if (!ctx) {
        ERROR("Failed to create parser context");
        free(source);
        free(output_file);
        return 1;
    }
    
    ast_node_t* ast = NULL;
    if (vibe_parse(ctx, &ast) && ast) {
        INFO("Parsing successful");
        
        if (verbose) {
            INFO("AST structure:");
            ast_print(ast, 0);
        }
        
        // Perform semantic analysis
        INFO("Performing semantic analysis...");
        if (semantic_analyze(ast)) {
            INFO("Semantic analysis successful");
            
            // Generate code
            INFO("Generating code...");
            if (generate_code(ast, output_file)) {
                INFO("Code generation successful");
                
                if (!compile_only) {
                    INFO("Compiling generated C code...");
                    // Compile the generated C code
                    char cmd[512];
                    snprintf(cmd, sizeof(cmd), "gcc -shared -fPIC %s -o %s.so", 
                            output_file, output_file);
                    
                    int result = system(cmd);
                    if (result == 0) {
                        INFO("Compilation successful: %s.so", output_file);
                    } else {
                        ERROR("C compilation failed");
                    }
                }
            } else {
                ERROR("Code generation failed");
            }
        } else {
            ERROR("Semantic analysis failed");
        }
        
        // Clean up semantic analyzer resources
        semantic_cleanup();
        
        // Clean up AST
        ast_node_free(ast);
    } else {
        ERROR("Parsing failed");
        vibe_destroy(ctx);
        free(source);
        free(output_file);
        return 1;
    }
    
    vibe_destroy(ctx);
    free(source);
    free(output_file);
    
    return 0;
}