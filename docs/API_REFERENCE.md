# VibeLang C API Reference

This document provides a comprehensive reference for the VibeLang C API, which allows you to integrate VibeLang modules into your C/C++ applications.

## Basic Usage

```c
#include <vibelang.h>

int main() {
    // Initialize VibeLang runtime
    vibelang_init();
    
    // Load a module
    VibeModule* module = vibelang_load("mymodule.vibe");
    
    // Call a function
    VibeValue* result = vibe_call(module, "my_function", 
                               vibe_value_string("argument1"), 
                               vibe_value_int(42));
    
    // Use the result
    printf("Result: %s\n", vibe_value_get_string(result));
    
    // Clean up
    vibe_value_free(result);
    vibelang_unload(module);
    vibelang_shutdown();
    
    return 0;
}
```

## Initialization and Cleanup

### `VibeError vibelang_init(void)`

Initializes the VibeLang runtime. This must be called before using any other VibeLang functions.

**Returns:** `VIBE_SUCCESS` on success, or an error code on failure.

### `void vibelang_shutdown(void)`

Shuts down the VibeLang runtime and frees all associated resources.

## Module Management

### `VibeModule* vibelang_load(const char* filename)`

Loads a VibeLang module from a file. The file can be either a .vibe source file or a compiled .so/.dll file.

**Parameters:**
- `filename`: Path to the VibeLang module file.

**Returns:** A handle to the loaded module, or NULL on error.

### `void vibelang_unload(VibeModule* module)`

Unloads a previously loaded module and frees all associated resources.

**Parameters:**
- `module`: The module to unload.

## Function Calling

### `VibeValue* vibe_call(VibeModule* module, const char* function_name, ...)`

Calls a function in a VibeLang module with variable arguments.

**Parameters:**
- `module`: The module containing the function.
- `function_name`: Name of the function to call.
- `...`: Variable arguments to pass to the function. Arguments must be created with `vibe_value_*` functions.

**Returns:** The function's return value, or NULL on error.

### `VibeValue* vibe_call_with_args(VibeModule* module, const char* function_name, VibeValue** args, size_t arg_count)`

Calls a function in a VibeLang module with an array of arguments.

**Parameters:**
- `module`: The module containing the function.
- `function_name`: Name of the function to call.
- `args`: Array of argument values.
- `arg_count`: Number of arguments.

**Returns:** The function's return value, or NULL on error.

## Value Creation

### `VibeValue* vibe_value_null(void)`

Creates a null value.

**Returns:** A new null value.

### `VibeValue* vibe_value_bool(int value)`

Creates a boolean value.

**Parameters:**
- `value`: Boolean value (0 for false, non-zero for true).

**Returns:** A new boolean value.

### `VibeValue* vibe_value_int(long long value)`

Creates an integer value.

**Parameters:**
- `value`: Integer value.

**Returns:** A new integer value.

### `VibeValue* vibe_value_float(double value)`

Creates a floating point value.

**Parameters:**
- `value`: Floating point value.

**Returns:** A new floating point value.

### `VibeValue* vibe_value_string(const char* value)`

Creates a string value.

**Parameters:**
- `value`: String value (will be copied).

**Returns:** A new string value.

## Value Access

### `VibeValueType vibe_value_get_type(const VibeValue* value)`

Gets the type of a value.

**Parameters:**
- `value`: The value to check.

**Returns:** The type of the value.

### `int vibe_value_get_bool(const VibeValue* value)`

Gets a boolean value.

**Parameters:**
- `value`: The value to get.

**Returns:** The boolean value, or 0 if not a boolean.

### `long long vibe_value_get_int(const VibeValue* value)`

Gets an integer value. Will convert from other numeric types if possible.

**Parameters:**
- `value`: The value to get.

**Returns:** The integer value, or 0 if not convertible.

### `double vibe_value_get_float(const VibeValue* value)`

Gets a floating point value. Will convert from other numeric types if possible.

**Parameters:**
- `value`: The value to get.

**Returns:** The floating point value, or 0.0 if not convertible.

### `const char* vibe_value_get_string(const VibeValue* value)`

Gets a string value.

**Parameters:**
- `value`: The value to get.

**Returns:** The string value, or NULL if not a string.

### `void vibe_value_free(VibeValue* value)`

Frees a value.

**Parameters:**
- `value`: The value to free.

## Error Handling

### `const char* vibe_get_error_message(void)`

Gets the last error message.

**Returns:** The last error message, or NULL if no error.

## Runtime Configuration

VibeLang runtime behavior can be configured using the `vibeconfig.json` file, which should be placed in the same directory as your application. The configuration file can specify the LLM provider, API key, and other parameters:

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
    "functionName": { 
      "temperature": 0.5,
      "max_tokens": 100
    }
  }
}
```

You can also set the API key using environment variables:
- `VIBELANG_API_KEY` - General API key for any provider
- `OPENAI_API_KEY` - Specific to OpenAI
- `ANTHROPIC_API_KEY` - Specific to Anthropic

## Advanced Usage: Direct Prompt Execution

For advanced use cases, you can directly execute prompts without going through a VibeLang module:

```c
#include <vibelang.h>
#include <vibelang/runtime.h>

int main() {
    vibelang_init();
    
    // Execute a prompt directly
    VibeValue* result = vibe_execute_prompt("What is the capital of France?");
    
    printf("Answer: %s\n", vibe_value_get_string(result));
    
    vibe_value_free(result);
    vibelang_shutdown();
    
    return 0;
}
```

## Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0 | `VIBE_SUCCESS` | Operation successful |
| 1 | `VIBE_ERROR_FILE_NOT_FOUND` | File not found |
| 2 | `VIBE_ERROR_COMPILATION_FAILED` | Compilation failed |
| 3 | `VIBE_ERROR_FUNCTION_NOT_FOUND` | Function not found in module |
| 4 | `VIBE_ERROR_TYPE_MISMATCH` | Type mismatch in function call |
| 5 | `VIBE_ERROR_LLM_CONNECTION_FAILED` | Failed to connect to LLM provider |
| 6 | `VIBE_ERROR_MEMORY_ALLOCATION` | Memory allocation failed |

## Value Types

| Type | Name | Description |
|------|------|-------------|
| 0 | `VIBE_TYPE_NULL` | Null value |
| 1 | `VIBE_TYPE_BOOL` | Boolean value |
| 2 | `VIBE_TYPE_INT` | Integer value |
| 3 | `VIBE_TYPE_FLOAT` | Floating point value |
| 4 | `VIBE_TYPE_STRING` | String value |
| 5 | `VIBE_TYPE_ARRAY` | Array of values |
| 6 | `VIBE_TYPE_OBJECT` | Object with key-value pairs |
