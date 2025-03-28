# VibeLang Implementation Details

This document describes the internal implementation details of VibeLang, intended for developers who wish to understand or modify the language implementation.

## Compiler Implementation

### Lexer and Parser

VibeLang uses [PackCC](https://github.com/arithy/packcc), a parser generator based on Parsing Expression Grammar (PEG). The grammar is defined in `src/compiler/grammar.peg` and compiled to C code.

The parser generates an Abstract Syntax Tree (AST) that represents the structure of the source code. Each node in the AST has a type, properties, and child nodes.

### AST Structure

The AST is implemented as a tree of `ast_node_t` structures, defined in `src/utils/ast.h`:

```c
typedef struct ast_node {
    ast_node_type_t type;
    
    /* Properties storage */
    struct {
        char* key;
        ast_val_type_t type;
        ast_val_t val;
    } *props;
    size_t prop_count;
    size_t prop_capacity;
    
    /* Children nodes */
    struct ast_node** children;
    size_t child_count;
    size_t child_capacity;
    
    /* Source location info */
    int line;
    int column;
} ast_node_t;
```

Properties are stored as key-value pairs and can have different types (int, float, string, bool).

### Semantic Analysis

Semantic analysis is performed in the `semantic_analyze` function in `src/compiler/semantic.c`. This phase includes:

1. Symbol table construction
2. Type checking
3. Meaning type validation
4. Error detection

The symbol table is implemented as a linked list of scopes, with each scope containing a linked list of symbols:

```c
typedef struct symbol_scope {
    symbol_t* symbols;  // Linked list of symbols in this scope
    struct symbol_scope* parent;  // Parent scope
    ast_node_t* node;  // AST node for this scope
} symbol_scope_t;
```

### Code Generation

Code generation is handled by `src/compiler/codegen.c`. The compiler generates C code that calls the VibeLang runtime library.

The code generation process:

1. Generates standard headers and includes
2. Processes type definitions and function prototypes
3. Generates function implementations
4. Handles prompt blocks by generating code for:
   - Template variable substitution
   - LLM API calls
   - Response processing

#### Prompt Block Code Generation

Prompt blocks are transformed into C code that:

1. Identifies variables in the current scope
2. Creates arrays of variable names and values
3. Calls the `format_prompt` function to substitute variables
4. Calls the `vibe_execute_prompt` function to send the prompt to the LLM
5. Converts the response to the appropriate return type

Example generated code for a prompt block:

```c
{
    VibeValue* prompt_result = NULL;
    const char* prompt_template = "What is the temperature in {city}?";
    int var_count = 1;
    char** var_names = malloc(sizeof(char*) * var_count);
    char** var_values = malloc(sizeof(char*) * var_count);
    var_names[0] = "city";
    var_values[0] = strdup(city ? city : "");
    char* formatted_prompt = format_prompt(prompt_template, var_names, var_values, var_count);
    prompt_result = vibe_execute_prompt(formatted_prompt);
    if (prompt_result) {
        // Convert LLM response to the appropriate return type
        return vibe_value_get_int(prompt_result);
    }
    // Free resources
    free(formatted_prompt);
    for (int i = 0; i < var_count; i++) {
        free(var_values[i]);
    }
    free(var_names);
    free(var_values);
    // Default return if prompt fails
    return 0;
}
```

## Runtime Implementation

### Module System

VibeLang modules are implemented as dynamically loaded shared libraries. The runtime provides functions to load, use, and unload these modules:

```c
VibeModule* vibelang_load(const char* filename);
void vibelang_unload(VibeModule* module);
```

The `VibeModule` structure contains:
- A handle to the loaded shared library
- The module name
- The file path

### Value System

Values in VibeLang are represented by the `VibeValue` structure, which uses a tagged union to store different types of values:

```c
struct VibeValue {
    VibeValueType type;
    union {
        int bool_val;
        long long int_val;
        double float_val;
        char* string_val;
        struct {
            VibeValue** items;
            size_t count;
        } array_val;
        struct {
            char** keys;
            VibeValue** values;
            size_t count;
        } object_val;
    } data;
};
```

The runtime provides functions to create, access, and free these values:

```c
VibeValue* vibe_value_int(long long value);
long long vibe_value_get_int(const VibeValue* value);
void vibe_value_free(VibeValue* value);
```

### LLM Integration

LLM integration is handled by the `src/runtime/llm_interface.c` file. It provides:

1. A provider-agnostic interface for LLM requests
2. Template variable substitution
3. HTTP requests using libcurl
4. Response parsing using cJSON

#### JSON Response Parsing

Responses from the OpenAI API are returned as JSON objects. VibeLang automatically parses these responses to extract the relevant content:

```c
static char* parse_openai_response(const char* json_str) {
  if (!json_str) {
    ERROR("NULL JSON response");
    return NULL;
  }
  
  cJSON* root = cJSON_Parse(json_str);
  if (!root) {
    ERROR("Failed to parse JSON response");
    return NULL;
  }
  
  // First, get the "choices" array
  cJSON* choices = cJSON_GetObjectItem(root, "choices");
  if (!choices || !cJSON_IsArray(choices)) {
    ERROR("Invalid choices array in response");
    cJSON_Delete(root);
    return NULL;
  }
  
  // Get the first choice
  cJSON* choice = cJSON_GetArrayItem(choices, 0);
  if (!choice) {
    ERROR("Failed to get first choice from response");
    cJSON_Delete(root);
    return NULL;
  }
  
  // Get the "message" object
  cJSON* message = cJSON_GetObjectItem(choice, "message");
  if (!message) {
    ERROR("Message object not found in choice");
    cJSON_Delete(root);
    return NULL;
  }
  
  // Get the "content" string
  cJSON* content = cJSON_GetObjectItem(message, "content");
  if (!content || !cJSON_IsString(content)) {
    ERROR("Content not found or not a string");
    cJSON_Delete(root);
    return NULL;
  }
  
  // Extract the content string
  char* result = strdup(content->valuestring);
  
  // Clean up JSON objects
  cJSON_Delete(root);
  
  return result;
}
```

#### Request Format

OpenAI API requests are formatted as JSON objects with the following structure:

```json
{
  "model": "gpt-3.5-turbo",
  "messages": [
    {
      "role": "user",
      "content": "The prompt text"
    }
  ],
  "temperature": 0.7
}
```

#### Development Mode

For testing purposes, VibeLang includes a development mode that can be enabled by setting the environment variable `VIBELANG_DEV_MODE=1`. In development mode, LLM requests are not actually sent to external APIs, but instead return predefined mock responses based on keywords in the prompt.

#### Configuration System

The runtime can be configured through the `vibeconfig.json` file, which is processed by `src/runtime/config.c`. The configuration includes:

- LLM provider selection (e.g., OpenAI)
- API keys
- Default parameters (model, temperature, etc.)
- Function-specific overrides

Example configuration:

```json
{
  "global": {
    "provider": "OpenAI",
    "api_key": "your-api-key",
    "default_params": { 
      "model": "gpt-4",
      "temperature": 0.7, 
      "max_tokens": 150 
    }
  },
  "overrides": {
    "getTemperature": { 
      "temperature": 0.5,
      "max_tokens": 100
    }
  }
}
```

### Caching System

To improve performance and reduce API calls, VibeLang provides a caching system in `src/utils/cache_utils.c`. The caching system:

1. Stores LLM responses based on the prompts
2. Configurable cache location and expiration
3. Uses a simple file-based cache by default

## Tools and Utilities

### Command Line Compiler

The `vibec` command line tool (`src/tools/vibec.c`) compiles VibeLang source files to C code and/or shared libraries. It:

1. Parses command line arguments
2. Reads input files
3. Processes them through the compiler pipeline
4. Generates output files
5. Optionally compiles the generated C code to a shared library

### Logging System

The logging system (`src/utils/log_utils.c`) provides:

1. Different log levels (DEBUG, INFO, WARNING, ERROR, FATAL)
2. File and line information
3. Colored output
4. Global log level configuration

## Testing Framework

The testing framework includes:

1. Unit tests for individual components
2. Integration tests for the complete pipeline
3. Generated code tests to verify correctness of the generated C code

Tests are implemented using a simple assertion-based approach and run using CTest from CMake.
