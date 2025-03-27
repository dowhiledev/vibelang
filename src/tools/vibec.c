#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "../utils/ast.h"
#include "../utils/log_utils.h"
#include "../compiler/parser.h"

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
    
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0) {
            compile_only = 1;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
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
        printf("Input file: %s\n", input_file);
        printf("Output file: %s\n", output_file);
    }
    
    // Placeholder for actual compilation process
    printf("Compiling %s to %s... (not yet implemented)\n", input_file, output_file);
    
    return 0;
}
