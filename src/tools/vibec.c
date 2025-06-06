/**
 * @file vibec.c
 * @brief Vibe language compiler command line tool
 */

#include "../../include/vibelang.h"
#include "../utils/file_utils.h"
#include "../utils/log_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int check_only;     // Just check syntax, don't generate output
  int verbose;        // Verbose output
  int help;           // Show help
  int version;        // Show version
  const char *input;  // Input file
  const char *output; // Output file
  int optimization;   // Optimization level (0-3)
} cli_options;

/**
 * Print usage information
 */
static void print_usage(const char *program_name) {
  printf("Usage: %s [options] input_file\n", program_name);
  printf("\nOptions:\n");
  printf("  -h, --help                Show this help message\n");
  printf("  -v, --version             Show version information\n");
  printf("  -o, --output <file>       Specify output file\n");
  printf(
      "  -c, --check               Only check syntax, don't generate output\n");
  printf("  -O<level>                 Optimization level (0-3)\n");
  printf("  --verbose                 Verbose output\n");
}

/**
 * Print version information
 */
static void print_version(void) {
  printf("VibeLanguage Compiler (vibec) version 0.1.0\n");
  printf("Copyright (C) 2023 VibeLanguage Team\n");
}

/**
 * Parse command line options
 */
static cli_options parse_options(int argc, char *argv[]) {
  cli_options options = {0};
  options.optimization = 0;

  // Need at least one argument (input file)
  if (argc < 2) {
    options.help = 1;
    return options;
  }

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      // Option
      if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
        options.help = 1;
      } else if (strcmp(argv[i], "-v") == 0 ||
                 strcmp(argv[i], "--version") == 0) {
        options.version = 1;
      } else if (strcmp(argv[i], "-c") == 0 ||
                 strcmp(argv[i], "--check") == 0) {
        options.check_only = 1;
      } else if (strcmp(argv[i], "--verbose") == 0) {
        options.verbose = 1;
      } else if (strcmp(argv[i], "-o") == 0 ||
                 strcmp(argv[i], "--output") == 0) {
        if (i + 1 < argc) {
          options.output = argv[++i];
        }
      } else if (argv[i][0] == '-' && argv[i][1] == 'O' && argv[i][2] != '\0') {
        options.optimization = atoi(&argv[i][2]);
        if (options.optimization < 0 || options.optimization > 3) {
          options.optimization = 0;
        }
      } else {
        fprintf(stderr, "Unknown option: %s\n", argv[i]);
        options.help = 1;
      }
    } else {
      // Input file
      if (options.input == NULL) {
        options.input = argv[i];
      } else {
        fprintf(stderr, "Multiple input files not supported\n");
        options.help = 1;
      }
    }
  }

  return options;
}

/**
 * Check if a file exists and is readable
 */
static int check_file_accessible(const char *filename) {
  if (!filename) {
    return 0;
  }

  FILE *file = fopen(filename, "r");
  if (!file) {
    return 0;
  }

  fclose(file);
  return 1;
}

/**
 * Check the syntax of a file without compiling
 */
static int check_syntax(const char *filename) {
  INFO("Checking syntax of file: %s", filename);

  // Make sure file exists and is readable
  if (!check_file_accessible(filename)) {
    ERROR("Cannot access file: %s", filename);
    return 1;
  }

  // Read the file contents
  char *source = read_file(filename);
  if (!source) {
    ERROR("Failed to read file: %s", filename);
    return 1;
  }

  // Parse the source and check for syntax errors
  ast_node_t *ast = NULL;
  int result = 0;

  // Initialize the compiler
  VibeError err = vibelang_init();
  if (err != VIBE_SUCCESS) {
    ERROR("Failed to initialize compiler");
    free(source);
    return 1;
  }

  // Set up a try/catch block using setjmp/longjmp for error handling
  INFO("Parsing file...");

  // Check if the source is valid
  if (!source || strlen(source) == 0) {
    ERROR("Empty or invalid source file");
    free(source);
    vibelang_shutdown();
    return 1;
  }

  // Try to parse the source (program-level syntax check)
  // Don't use parse_string directly, as it might not be exported correctly
  // Instead use the public API to compile but stop before code generation
  result = vibelang_compile(source, NULL);

  if (result != 0) {
    ERROR("Syntax check failed");
    free(source);
    vibelang_shutdown();
    return 1;
  }

  INFO("Syntax check passed");
  free(source);
  vibelang_shutdown();
  return 0;
}

/**
 * Main entry point for the compiler
 */
int main(int argc, char *argv[]) {
  // Initialize logging
  init_logging(LOG_LEVEL_INFO);

  // Parse command line options
  cli_options options = parse_options(argc, argv);

  // Show help if requested or no input file provided
  if (options.help || options.input == NULL) {
    print_usage(argv[0]);
    return options.input == NULL ? 1 : 0;
  }

  // Show version information if requested
  if (options.version) {
    print_version();
    return 0;
  }

  // Print info about what we're doing
  INFO("VibeLang library loaded");

  if (options.verbose) {
    set_log_level(LOG_LEVEL_DEBUG);
    DEBUG("Verbose mode enabled");
    DEBUG("Input file: %s", options.input);
    if (options.output) {
      DEBUG("Output file: %s", options.output);
    }
    DEBUG("Optimization level: %d", options.optimization);
  }

  // Check only mode - just validate syntax
  if (options.check_only) {
    return check_syntax(options.input);
  }

  // Get output filename if not specified
  char *output_file = NULL;
  if (options.output) {
    output_file = strdup(options.output);
  } else {
    // Replace .vibe extension with .c
    size_t input_len = strlen(options.input);
    if (input_len > 5 && strcmp(options.input + input_len - 5, ".vibe") == 0) {
      output_file = malloc(input_len);
      if (!output_file) {
        ERROR("Memory allocation failed");
        return 1;
      }
      strncpy(output_file, options.input, input_len - 5);
      output_file[input_len - 5] = '\0';
      strcat(output_file, ".c");
    } else {
      output_file = malloc(input_len + 3);
      if (!output_file) {
        ERROR("Memory allocation failed");
        return 1;
      }
      strcpy(output_file, options.input);
      strcat(output_file, ".c");
    }
  }

  // Read input file
  char *source = read_file(options.input);
  if (!source) {
    ERROR("Failed to read input file: %s", options.input);
    free(output_file);
    return 1;
  }

  // Initialize VibeLanguage
  VibeError err = vibelang_init();
  if (err != VIBE_SUCCESS) {
    ERROR("Failed to initialize VibeLanguage: %d", err);
    free(source);
    free(output_file);
    return 1;
  }

  // Compile the source
  int compile_result = vibelang_compile(source, output_file);
  if (compile_result != 0) {
    ERROR("Compilation failed");
    vibelang_shutdown();
    free(source);
    free(output_file);
    return 1;
  }

  INFO("Compilation successful, output written to %s", output_file);

  // Also build a shared library for runtime loading
  size_t out_len = strlen(output_file);
  char *lib_file = malloc(out_len + 4); // room for replacing .c with .so
  if (!lib_file) {
    ERROR("Memory allocation failed");
    vibelang_shutdown();
    free(source);
    free(output_file);
    return 1;
  }
  strcpy(lib_file, output_file);
  if (out_len > 2 && strcmp(&output_file[out_len - 2], ".c") == 0) {
    lib_file[out_len - 2] = '\0';
  }
  strcat(lib_file, ".so");

  char cmd[512];
  snprintf(cmd, sizeof(cmd),
           "gcc -shared -fPIC %s -o %s -lvibelang",
           output_file, lib_file);
  INFO("Building shared library %s", lib_file);
  if (options.verbose) {
    INFO("Running: %s", cmd);
  }
  if (system(cmd) != 0) {
    WARNING("Failed to build shared library with gcc");
  } else {
    INFO("Shared library created at %s", lib_file);
  }
  free(lib_file);

  // Cleanup
  vibelang_shutdown();
  free(source);
  free(output_file);

  return 0;
}
