#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/runtime.h"
#include "../../include/vibelang.h"
#include "../utils/file_utils.h"
#include "../utils/log_utils.h"
#include "config.h"        // Added missing header
#include "llm_interface.h" // Added missing header

// Track whether the runtime has been initialized
static int runtime_initialized = 0;

static void auto_shutdown(void) {
  if (runtime_initialized) {
    vibe_runtime_shutdown();
  }
}

// Internal runtime structures that extend the public ones
typedef struct {
  VibeModule base; // Include the public struct members
  void *handle;    // Handle to the dynamically loaded module
  char *filepath;  // Path to the .so file
} VibeModuleInternal;

// Keep the VibeError enum values in sync
typedef enum {
  VIBE_RUNTIME_ERROR_LLM_CONNECTION_FAILED = -100,
  VIBE_RUNTIME_ERROR_INVALID_MODULE = -101,
  VIBE_RUNTIME_ERROR_FUNCTION_NOT_FOUND = -102
} VibeRuntimeError;

// Function to initialize the LLM runtime
VibeError vibe_runtime_init() {
  if (runtime_initialized)
    return VIBE_SUCCESS;

  INFO("Initializing Vibe language runtime");

  // Load configuration
  if (!load_config()) {
    ERROR("Failed to load runtime configuration");
    return VIBE_ERROR_RUNTIME;
  }

  // Verify API key is present
  const char *api_key = get_api_key();
  if (!api_key || strlen(api_key) == 0) {
    ERROR("LLM API key not set");
    return VIBE_ERROR_RUNTIME;
  }

  // Initialize LLM connection
  if (!init_llm_connection()) {
    ERROR("Failed to initialize LLM connection");
    return VIBE_ERROR_LLM_CONNECTION_FAILED;
  }

  INFO("Vibe language runtime initialized successfully");
  runtime_initialized = 1;
  atexit(auto_shutdown);
  return VIBE_SUCCESS;
}

// Function to shut down the runtime
void vibe_runtime_shutdown() {
  if (!runtime_initialized)
    return;

  INFO("Shutting down Vibe language runtime");

  // Close LLM connection
  close_llm_connection();

  // Cleanup resources
  free_config();

  runtime_initialized = 0;
  INFO("Vibe language runtime shut down successfully");
}

// Function to execute a prompt-based function
VibeValue vibe_execute_prompt(const char *prompt, const char *meaning) {
  VibeValue result;

  // Initialize result as NULL in case of failure
  result.type = VIBE_NULL;

  if (!runtime_initialized) {
    if (vibe_runtime_init() != VIBE_SUCCESS) {
      ERROR("Runtime initialization failed");
      return result;
    }
  }

  if (!prompt) {
    ERROR("Invalid prompt parameter");
    return result;
  }

  DEBUG("Executing LLM prompt: %s (meaning: %s)", prompt, meaning);

  // Send the prompt to the LLM
  char *llm_response = send_llm_prompt(prompt, meaning);
  if (!llm_response) {
    ERROR("Failed to get response from LLM");
    return result;
  }

  // Parse the response based on the meaning
  if (meaning && strcmp(meaning, "temperature in Celsius") == 0) {
    // Parse as a number
    double temperature = atof(llm_response);
    result.type = VIBE_NUMBER;
    result.data.number_val = temperature;
    DEBUG("Parsed temperature: %f", temperature);
  } else if (meaning && strcmp(meaning, "weather description") == 0) {
    // Parse as a string
    result.type = VIBE_STRING;
    result.data.string_val = llm_response; // Transfer ownership
    DEBUG("Parsed weather description: %s", llm_response);
  } else {
    // Default to string
    result.type = VIBE_STRING;
    result.data.string_val = llm_response; // Transfer ownership
    DEBUG("Parsed as generic string: %s", llm_response);
  }

  return result;
}

// Function to load a module
VibeModule *vibe_load_module(const char *module_name) {
  if (!module_name) {
    ERROR("Invalid module name");
    return NULL;
  }

  // Create module path
  char module_path[512];
  snprintf(module_path, sizeof(module_path), "%s.vibe", module_name);

  // Check if the module file exists
  if (!file_exists(module_path)) {
    ERROR("Module file not found: %s", module_path);
    return NULL;
  }

  // Compile the module to a shared object if needed
  char so_path[512];
  snprintf(so_path, sizeof(so_path), "%s.so", module_name);

  // Check if the .so needs to be rebuilt
  if (!file_exists(so_path) ||
      get_file_mtime(module_path) > get_file_mtime(so_path)) {
    INFO("Compiling module: %s", module_name);

    // TODO: Implement the actual compilation
    // For now, just pretend we compiled it
    sleep(1);
  }

  // Load the shared object
  void *handle = dlopen(so_path, RTLD_LAZY);
  if (!handle) {
    ERROR("Failed to load module: %s", dlerror());
    return NULL;
  }

  // Create and initialize module
  VibeModuleInternal *mod_internal =
      (VibeModuleInternal *)malloc(sizeof(VibeModuleInternal));
  if (!mod_internal) {
    ERROR("Failed to allocate memory for module");
    dlclose(handle);
    return NULL;
  }

  // Initialize the public part
  VibeModule *module = &mod_internal->base;
  module->name = strdup(module_name);
  module->source_path = strdup(module_path);
  module->output_path = strdup(so_path);
  module->internal_data = NULL;

  // Initialize the private part
  mod_internal->handle = handle;
  mod_internal->filepath = strdup(so_path);

  INFO("Module loaded successfully: %s", module_name);
  return module;
}

// Function to unload a module
void vibe_unload_module(VibeModule *module) {
  if (!module)
    return;

  // Cast to internal structure
  VibeModuleInternal *mod_internal = (VibeModuleInternal *)module;

  INFO("Unloading module: %s", module->name);

  // Close the shared object
  if (mod_internal->handle) {
    dlclose(mod_internal->handle);
  }

  // Free allocated strings
  free(module->name);
  free(module->source_path);
  free(module->output_path);
  free(mod_internal->filepath);

  // Free module
  free(mod_internal);
}

// Function to call a function within a module
VibeValue *vibe_call_function(VibeModule *module, const char *function_name,
                              VibeValue *args, int arg_count) {
  if (!module || !function_name) {
    ERROR("Invalid module or function name");
    // Create a static error value to return
    static VibeValue error_value;
    error_value = vibe_string_value("Error: Invalid parameters");
    return &error_value;
  }

  // Cast to internal structure
  VibeModuleInternal *mod_internal = (VibeModuleInternal *)module;

  // Get function pointer
  void *func_ptr = dlsym(mod_internal->handle, function_name);
  if (!func_ptr) {
    ERROR("Function not found: %s", function_name);
    // Create a static error value to return
    static VibeValue error_value;
    error_value = vibe_string_value("Error: Function not found");
    return &error_value;
  }

  // Call function
  // This is just a placeholder - in reality, we'd need a more complex mechanism
  // to handle different function signatures
  INFO("Calling function: %s", function_name);

  // Return a dummy value for now
  static VibeValue result;
  result = vibe_null_value();
  return &result;
}

// Create a NULL value
VibeValue vibe_null_value() {
  VibeValue value;
  value.type = VIBE_NULL;
  return value;
}

// Create a boolean value
VibeValue vibe_bool_value(int b) {
  VibeValue val;
  val.type = VIBE_BOOLEAN;
  val.data.bool_val = b;
  return val;
}

// Create an integer value
VibeValue vibe_int_value(int value) {
  VibeValue val;
  val.type = VIBE_NUMBER;
  val.data.number_val = value;
  return val;
}

// Create a float value
VibeValue vibe_float_value(double value) {
  VibeValue val;
  val.type = VIBE_NUMBER;
  val.data.number_val = value;
  return val;
}

// Create a string value - note this implementation matches our header
// definition
VibeValue vibe_string_value(const char *str) {
  VibeValue value;
  value.type = VIBE_STRING;
  value.data.string_val = str ? strdup(str) : NULL;
  return value;
}

// Get string value from VibeValue
const char *vibe_get_string(VibeValue *value) {
  if (!value || value->type != VIBE_STRING || !value->data.string_val) {
    return "";
  }
  return value->data.string_val;
}

// Get number value from VibeValue
double vibe_get_number(VibeValue *value) {
  if (!value || value->type != VIBE_NUMBER) {
    return 0.0;
  }
  return value->data.number_val;
}

// Get boolean value from VibeValue
int vibe_get_bool(VibeValue *value) {
  if (!value || value->type != VIBE_BOOLEAN) {
    return 0;
  }
  return value->data.bool_val;
}

// Get integer value from VibeValue
int vibe_value_get_int(VibeValue *value) {
  if (!value)
    return 0;

  switch (value->type) {
  case VIBE_NUMBER:
    return (int)value->data.number_val;
  case VIBE_STRING:
    if (value->data.string_val)
      return atoi(value->data.string_val);
    break;
  case VIBE_BOOLEAN:
    return value->data.bool_val;
  case VIBE_NULL:
  default:
    break;
  }
  return 0;
}
