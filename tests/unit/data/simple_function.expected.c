// Generated by VibeLang Compiler

#include "runtime.h"
#include "vibelang.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for runtime functions
extern VibeValue vibe_execute_prompt(const char *prompt, const char *meaning);
extern char *format_prompt(const char *template, char **var_names,
                           char **var_values, int var_count);

// Temperature: temperature in Celsius
typedef int Temperature;

int getTemperature(const char *city);

int getTemperature(const char *city) {
  // LLM Prompt: What is the temperature in {city}?
  {
    VibeValue prompt_result;
    const char *prompt_template = "What is the temperature in {city}?";
    int var_count = 1;
    char **var_names = malloc(sizeof(char *) * var_count);
    char **var_values = malloc(sizeof(char *) * var_count);
    var_names[0] = "city";
    var_values[0] = strdup(city ? city : "");
    char *formatted_prompt =
        format_prompt(prompt_template, var_names, var_values, var_count);
    prompt_result = vibe_execute_prompt(formatted_prompt, "temperature in Celsius");
    // Free resources
    free(formatted_prompt);
    for (int i = 0; i < var_count; i++) {
      free(var_values[i]);
    }
    free(var_names);
    free(var_values);
    // Convert LLM response to the appropriate return type
    return vibe_value_get_int(&prompt_result);
  }
}
