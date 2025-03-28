# VibeLang Developer Guide

This document provides detailed information about VibeLang's architecture, implementation, and development workflow for contributors.

## Project Architecture

VibeLang is structured as a multi-layered system:

```ascii
+-------------------+
|   User Programs   |
+-------------------+
          |
   +------v------+
   |  Compiler   |
   +------+------+
          |
  +-------v-------+
  |      C        |
  |  Generation   |
  +-------+-------+
          |
    +-----v------+
    |  Runtime   |
    +-----+------+
          |
  +-------v---------+
  |  LLM Interface  |
  +-----------------+
```

### Core Components

#### 1. Compiler (`src/compiler/`)
- **Lexical Analyzer**: Defined in the grammar.peg file and processed by PackCC
- **Parser**: Generated from grammar.peg by PackCC, builds an AST representation
- **AST**: Abstract Syntax Tree representing the source code structure
- **Semantic Analyzer**: Type checking and symbol resolution
- **Code Generator**: Converts AST into C code

#### 2. Runtime (`src/runtime/`)
- **Module Loader**: Dynamically loads compiled VibeLang modules
- **LLM Interface**: Communicates with language model providers
- **Configuration System**: Manages API keys and LLM settings

#### 3. Utilities (`src/utils/`)
- **AST Utilities**: Functions for working with the AST
- **Logging**: Utilities for diagnostic output
- **File Handling**: File I/O and path manipulation

## Compilation Pipeline

1. **Parsing**: Source code → AST
   - Grammar defined in PEG (Parsing Expression Grammar) format
   - PackCC generates a recursive descent parser

2. **Semantic Analysis**: AST → Validated AST
   - Symbol resolution and type checking
   - Meaning type validation
   - Duplicate symbol detection
   - Error detection

3. **Code Generation**: Validated AST → C code
   - Type mapping (VibeLang → C)
   - Prompt block translation with variable extraction
   - Return type handling for prompt blocks
   - Runtime API calls injection

4. **Compilation**: C code → Shared library
   - GCC/Clang compiles generated C code
   - Creates .so/.dll that can be dynamically loaded

## Prompt Handling

When a prompt block is encountered:

1. Variables in scope are identified (like `{name}` in the template)
2. The code generator extracts variable names from the template
3. Code to create variable name and value arrays is generated
4. Template substitution code is generated using `format_prompt`
5. Runtime call to `vibe_execute_prompt()` is inserted
6. Response conversions to the appropriate return type are added based on the function's return type

### Example:

From a VibeLang prompt block:
```vibe
prompt "What is the weather like in {city} on {date}?";
```

The compiler generates:
```c
{
    // Prompt block: "What is the weather like in {city} on {date}?"
    const char* prompt_template = "What is the weather like in {city} on {date}?";
    int var_count = 2;
    char** var_names = malloc(sizeof(char*) * var_count);
    char** var_values = malloc(sizeof(char*) * var_count);
    var_names[0] = "city";
    var_values[0] = city ? strdup(city) : strdup("");
    var_names[1] = "date";
    var_values[1] = date ? strdup(date) : strdup("");
    char* formatted_prompt = format_prompt(prompt_template, var_names, var_values, var_count);
    VibeValue* prompt_result = vibe_execute_prompt(formatted_prompt, prompt_template);
    
    // Free resources
    free(formatted_prompt);
    for (int i = 0; i < var_count; i++) {
        free(var_values[i]);
    }
    free(var_names);
    free(var_values);
    
    // Return the result with the appropriate type
    return vibe_value_get_string(prompt_result);
}
```

## Building and Packaging

### RPATH and Dynamic Library Handling

For proper runtime library resolution:

- On macOS, we use `@rpath` and `@executable_path` to find libraries relative to the executable
- The installer includes an `install_name_tool` step to update dynamic references
- CMake is configured with proper RPATH settings

```cmake
# Add to CMakeLists.txt for proper RPATH handling
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
if(APPLE)
  set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/../lib;${CMAKE_INSTALL_RPATH}")
  set(CMAKE_INSTALL_NAME_DIR "@rpath")
  set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR TRUE)
endif()
```

### Cross-Platform Compatibility

- Windows: Dynamic libraries use `.dll` extension
- macOS: Dynamic libraries use `.dylib` extension
- Linux: Dynamic libraries use `.so` extension

## Testing

The project uses a comprehensive test suite:

- **Unit Tests**: Test individual components in isolation
- **Integration Tests**: Test the full compilation pipeline
- **Generated Code Tests**: Verify the correctness of generated C code

Tests are organized by component:
- `test_ast.c`: Tests AST creation and manipulation
- `test_parser_utils.c`: Tests parser utility functions
- `test_semantic.c`: Tests semantic analysis
- `test_codegen.c`: Tests code generation
- `test_bison_parser.c`: Tests the Bison parser integration

To run tests:
```bash
cd build
make test
```

## Common Development Tasks

### Adding a New Feature

1. Update the grammar in `grammar.peg`
2. Update the AST node types in `ast.h`
3. Add semantic analysis in `semantic.c`
4. Add code generation in `codegen.c`
5. Add runtime support if needed
6. Add tests

### Fixing Common Issues

- **RPATH Issues**: On macOS, make sure to use `install_name_tool` to update library paths
- **Symbol Resolution**: Ensure `parser_bison.h` is present for compatibility with legacy code
- **Memory Management**: Check for proper cleanup in all code paths, especially in semantic analysis and code generation
- **Missing Libraries**: Add required libraries to CMakeLists.txt

## Documentation

The VibeLang project uses Doxygen for code documentation. To generate the documentation:

### Prerequisites

- Install Doxygen: 
  - On Ubuntu: `sudo apt-get install doxygen graphviz`
  - On macOS: `brew install doxygen graphviz`
  - On Windows: Download from the [Doxygen website](https://www.doxygen.nl/download.html)

### Generating Documentation

Run the following command from the project root directory:

```bash
make docs
```

The generated documentation can be found in `docs/generated/html/index.html`.

### Documentation Standards

When documenting code, please follow these standards:

1. Use Doxygen-style comments for functions, classes, and types:
   ```c
   /**
    * @brief Brief description
    *
    * Detailed description...
    *
    * @param param1 Description of param1
    * @param param2 Description of param2
    * @return Description of the return value
    */
   ```

2. For simple comments within functions, use standard C comments:
   ```c
   // This is a simple comment
   int x = 5; // End-of-line comment
   ```

3. Add examples where appropriate:
   ```c
   /**
    * @brief Converts Celsius to Fahrenheit
    * 
    * @param celsius Temperature in Celsius
    * @return Temperature in Fahrenheit
    * 
    * @example
    * double fahrenheit = celsius_to_fahrenheit(25.0);
    * // fahrenheit = 77.0
    */
   ```
