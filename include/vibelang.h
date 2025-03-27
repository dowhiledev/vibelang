/**
 * VibeLang Public C API
 * 
 * This header defines the public interface for integrating with VibeLang
 * from C applications.
 */

#ifndef VIBELANG_H
#define VIBELANG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Type definitions */
typedef struct VibeModule VibeModule;
typedef struct VibeValue VibeValue;

/* Error codes */
typedef enum {
    VIBE_SUCCESS = 0,
    VIBE_ERROR_FILE_NOT_FOUND,
    VIBE_ERROR_COMPILATION_FAILED,
    VIBE_ERROR_FUNCTION_NOT_FOUND,
    VIBE_ERROR_TYPE_MISMATCH,
    VIBE_ERROR_LLM_CONNECTION_FAILED,
    VIBE_ERROR_MEMORY_ALLOCATION
} VibeError;

/* Value types */
typedef enum {
    VIBE_TYPE_NULL,
    VIBE_TYPE_BOOL,
    VIBE_TYPE_INT,
    VIBE_TYPE_FLOAT,
    VIBE_TYPE_STRING,
    VIBE_TYPE_ARRAY,
    VIBE_TYPE_OBJECT
} VibeValueType;

/* Module loading/unloading */
VibeError vibelang_init(void);
void vibelang_shutdown(void);

VibeModule* vibelang_load(const char* filename);
void vibelang_unload(VibeModule* module);

/* Function calling */
VibeValue* vibe_call(VibeModule* module, const char* function_name, ...);
VibeValue* vibe_call_with_args(VibeModule* module, const char* function_name, VibeValue** args, size_t arg_count);

/* Value creation and access */
VibeValue* vibe_value_null(void);
VibeValue* vibe_value_bool(int value);
VibeValue* vibe_value_int(long long value);
VibeValue* vibe_value_float(double value);
VibeValue* vibe_value_string(const char* value);

VibeValueType vibe_value_get_type(const VibeValue* value);
int vibe_value_get_bool(const VibeValue* value);
long long vibe_value_get_int(const VibeValue* value);
double vibe_value_get_float(const VibeValue* value);
const char* vibe_value_get_string(const VibeValue* value);

void vibe_value_free(VibeValue* value);

/* Error handling */
const char* vibe_get_error_message(void);

#ifdef __cplusplus
}
#endif

#endif /* VIBELANG_H */
