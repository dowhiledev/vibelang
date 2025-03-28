/**
 * @file llm_interface.h
 * @brief Interface for LLM API communications
 */

#ifndef LLM_INTERFACE_H
#define LLM_INTERFACE_H

#include "../../include/vibelang.h"

/**
 * Initialize the LLM connection
 * 
 * @return 1 on success, 0 on failure
 */
int init_llm_connection(void);

/**
 * Send a prompt to the LLM and get the response
 * 
 * @param prompt The formatted prompt to send
 * @param meaning The semantic meaning of the prompt (used for response processing)
 * @return The response from the LLM, or NULL on error
 */
char* send_llm_prompt(const char* prompt, const char* meaning);

/**
 * Close the LLM connection
 */
void close_llm_connection(void);

/**
 * Format a prompt template with variable values
 * 
 * @param template The prompt template with variables in {variable} format
 * @param var_names Array of variable names
 * @param var_values Array of variable values
 * @param var_count Number of variables
 * @return The formatted prompt
 */
char* format_prompt(const char* template, char** var_names, char** var_values, int var_count);

#endif /* LLM_INTERFACE_H */
