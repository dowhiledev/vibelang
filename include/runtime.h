#ifndef VIBELANG_RUNTIME_H
#define VIBELANG_RUNTIME_H

#include <stddef.h>
#include "vibelang.h"

#ifdef __cplusplus
extern "C" {
#endif

/* LLM Configuration */
int load_config_from_file(const char* filename);
void cleanup_config();
const char* get_llm_provider();
const char* get_llm_api_key();

/* LLM Interface */
int init_llm_interface();
void cleanup_llm_interface();
char* execute_prompt(const char* prompt, const char* function_name);
char* format_prompt(const char* template, char** var_names, char** var_values, int var_count);

/* Prompt execution */
VibeValue* vibe_execute_prompt(const char* prompt);

#ifdef __cplusplus
}
#endif

#endif /* VIBELANG_RUNTIME_H */
