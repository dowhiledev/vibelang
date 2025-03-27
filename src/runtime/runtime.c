#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "../../include/vibelang.h"
#include "../utils/log_utils.h"
#include "../utils/file_utils.h"
#include "../utils/cache_utils.h"

/* Forward declarations */
extern int load_config_from_file(const char* filename);
extern void cleanup_config();
extern int init_llm_interface();
extern void cleanup_llm_interface();
extern char* execute_prompt(const char* prompt, const char* function_name);

/* VibeModule structure implementation */
struct VibeModule {
    void* handle;          // dlopen handle
    char* name;            // Module name
    char* filepath;        // Path to the shared library
};

/* VibeValue structure implementation */
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

/* Global error message */
static char error_message[1024] = "";

/* Initialize the VibeLang runtime */
VibeError vibelang_init() {
    INFO("Initializing VibeLang runtime");
    
    // Initialize cache
    cache_init(NULL);
    
    // Load configuration
    if (!load_config_from_file("vibeconfig.json")) {
        WARN("Failed to load configuration file, using defaults");
    }
    
    // Initialize LLM interface
    if (!init_llm_interface()) {
        ERROR("Failed to initialize LLM interface");
        return VIBE_ERROR_LLM_CONNECTION_FAILED;
    }
    
    return VIBE_SUCCESS;
}

/* Shut down the VibeLang runtime */
void vibelang_shutdown() {
    INFO("Shutting down VibeLang runtime");
    
    cleanup_llm_interface();
    cleanup_config();
    cache_cleanup();
}

/* Load a VibeLang module */
VibeModule* vibelang_load(const char* filename) {
    if (!filename) {
        snprintf(error_message, sizeof(error_message), "No filename provided");
        return NULL;
    }
    
    // Check if file exists
    if (!file_exists(filename)) {
        snprintf(error_message, sizeof(error_message), "File not found: %s", filename);
        return NULL;
    }
    
    // Check if we need to compile the file
    char* base_name = NULL;
    const char* extension = get_file_extension(filename);
    
    if (extension && strcmp(extension, "vibe") == 0) {
        INFO("Compiling VibeLang file: %s", filename);
        
        // Extract base name
        char* filename_copy = strdup(filename);
        char* dot = strrchr(filename_copy, '.');
        if (dot) *dot = '\0';
        base_name = strdup(filename_copy);
        free(filename_copy);
        
        // Compile to shared library
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "vibec -o %s.c %s", base_name, filename);
        int result = system(cmd);
        if (result != 0) {
            snprintf(error_message, sizeof(error_message), "Failed to compile %s", filename);
            free(base_name);
            return NULL;
        }
        
        snprintf(cmd, sizeof(cmd), "gcc -shared -fPIC %s.c -o %s.so", base_name, base_name);
        result = system(cmd);
        if (result != 0) {
            snprintf(error_message, sizeof(error_message), "Failed to create shared library for %s", filename);
            free(base_name);
            return NULL;
        }
    } else {
        // Assume it's already a shared library
        base_name = strdup(filename);
    }
    
    // Construct path to shared library
    char* so_path;
    if (extension && strcmp(extension, "so") == 0) {
        so_path = strdup(filename);
    } else {
        so_path = malloc(strlen(base_name) + 4);
        sprintf(so_path, "%s.so", base_name);
    }
    
    // Open shared library
    void* handle = dlopen(so_path, RTLD_LAZY);
    if (!handle) {
        snprintf(error_message, sizeof(error_message), "Failed to load shared library: %s", dlerror());
        free(base_name);
        free(so_path);
        return NULL;
    }
    
    // Create module structure
    VibeModule* module = malloc(sizeof(VibeModule));
    if (!module) {
        snprintf(error_message, sizeof(error_message), "Memory allocation failed");
        dlclose(handle);
        free(base_name);
        free(so_path);
        return NULL;
    }
    
    module->handle = handle;
    module->name = base_name;
    module->filepath = so_path;
    
    INFO("Loaded module: %s", module->name);
    return module;
}

/* Unload a VibeLang module */
void vibelang_unload(VibeModule* module) {
    if (!module) return;
    
    INFO("Unloading module: %s", module->name);
    
    if (module->handle) {
        dlclose(module->handle);
    }
    
    free(module->name);
    free(module->filepath);
    free(module);
}

/* Call a function in a VibeLang module */
VibeValue* vibe_call(VibeModule* module, const char* function_name, ...) {
    if (!module || !function_name) {
        snprintf(error_message, sizeof(error_message), "Invalid module or function name");
        return NULL;
    }
    
    // Look up function symbol
    void* func_ptr = dlsym(module->handle, function_name);
    if (!func_ptr) {
        snprintf(error_message, sizeof(error_message), "Function not found: %s", function_name);
        return NULL;
    }
    
    // TODO: Handle variable arguments by introspecting the function signature
    // For now, just call without arguments and return a null value
    
    INFO("Function call not yet implemented: %s", function_name);
    return vibe_value_null();
}

/* Create value objects */
VibeValue* vibe_value_null() {
    VibeValue* value = malloc(sizeof(VibeValue));
    if (!value) {
        snprintf(error_message, sizeof(error_message), "Memory allocation failed");
        return NULL;
    }
    
    value->type = VIBE_TYPE_NULL;
    return value;
}

VibeValue* vibe_value_bool(int value) {
    VibeValue* val = malloc(sizeof(VibeValue));
    if (!val) {
        snprintf(error_message, sizeof(error_message), "Memory allocation failed");
        return NULL;
    }
    
    val->type = VIBE_TYPE_BOOL;
    val->data.bool_val = value;
    return val;
}

VibeValue* vibe_value_int(long long value) {
    VibeValue* val = malloc(sizeof(VibeValue));
    if (!val) {
        snprintf(error_message, sizeof(error_message), "Memory allocation failed");
        return NULL;
    }
    
    val->type = VIBE_TYPE_INT;
    val->data.int_val = value;
    return val;
}

VibeValue* vibe_value_float(double value) {
    VibeValue* val = malloc(sizeof(VibeValue));
    if (!val) {
        snprintf(error_message, sizeof(error_message), "Memory allocation failed");
        return NULL;
    }
    
    val->type = VIBE_TYPE_FLOAT;
    val->data.float_val = value;
    return val;
}

VibeValue* vibe_value_string(const char* value) {
    if (!value) return vibe_value_null();
    
    VibeValue* val = malloc(sizeof(VibeValue));
    if (!val) {
        snprintf(error_message, sizeof(error_message), "Memory allocation failed");
        return NULL;
    }
    
    val->type = VIBE_TYPE_STRING;
    val->data.string_val = strdup(value);
    return val;
}

/* Get value properties */
VibeValueType vibe_value_get_type(const VibeValue* value) {
    if (!value) return VIBE_TYPE_NULL;
    return value->type;
}

int vibe_value_get_bool(const VibeValue* value) {
    if (!value || value->type != VIBE_TYPE_BOOL) return 0;
    return value->data.bool_val;
}

long long vibe_value_get_int(const VibeValue* value) {
    if (!value) return 0;
    
    switch (value->type) {
        case VIBE_TYPE_INT:
            return value->data.int_val;
        case VIBE_TYPE_FLOAT:
            return (long long)value->data.float_val;
        case VIBE_TYPE_BOOL:
            return value->data.bool_val ? 1 : 0;
        default:
            return 0;
    }
}

double vibe_value_get_float(const VibeValue* value) {
    if (!value) return 0.0;
    
    switch (value->type) {
        case VIBE_TYPE_FLOAT:
            return value->data.float_val;
        case VIBE_TYPE_INT:
            return (double)value->data.int_val;
        case VIBE_TYPE_BOOL:
            return value->data.bool_val ? 1.0 : 0.0;
        default:
            return 0.0;
    }
}

const char* vibe_value_get_string(const VibeValue* value) {
    if (!value || value->type != VIBE_TYPE_STRING) return NULL;
    return value->data.string_val;
}

/* Free a value object */
void vibe_value_free(VibeValue* value) {
    if (!value) return;
    
    // Clean up resources based on type
    if (value->type == VIBE_TYPE_STRING && value->data.string_val) {
        free(value->data.string_val);
    } else if (value->type == VIBE_TYPE_ARRAY) {
        for (size_t i = 0; i < value->data.array_val.count; i++) {
            vibe_value_free(value->data.array_val.items[i]);
        }
        free(value->data.array_val.items);
    } else if (value->type == VIBE_TYPE_OBJECT) {
        for (size_t i = 0; i < value->data.object_val.count; i++) {
            free(value->data.object_val.keys[i]);
            vibe_value_free(value->data.object_val.values[i]);
        }
        free(value->data.object_val.keys);
        free(value->data.object_val.values);
    }
    
    free(value);
}

/* Get the last error message */
const char* vibe_get_error_message() {
    return error_message;
}
