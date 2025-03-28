/**
 * @file llm_interface.h
 * @brief LLM (Language Model) interface functions for the Vibe language runtime
 */

#ifndef LLM_INTERFACE_H
#define LLM_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the connection to the LLM service
 * 
 * @return 1 on success, 0 on failure
 */
int init_llm_connection(void);

/**
 * Close the connection to the LLM service
 */
void close_llm_connection(void);

/**
 * Send a prompt to the LLM with a specific meaning context
 * 
 * @param prompt The text prompt to send
 * @param meaning The semantic meaning context
 * @return The response from the LLM, or NULL on error (caller must free)
 */
char* send_llm_prompt(const char* prompt, const char* meaning);

#ifdef __cplusplus
}
#endif

#endif /* LLM_INTERFACE_H */
