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

void useTemp();

void useTemp() {
  int temp = 25;
}
