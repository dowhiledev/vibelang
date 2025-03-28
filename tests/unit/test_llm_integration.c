#include "../../include/runtime.h"
#include "../../include/vibelang.h"
#include "../../src/runtime/llm_interface.h"
#include "../../src/utils/log_utils.h"
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Global flag to track which test is currently running
static const char *current_test = "None";

// Signal handler for detecting segfaults
static void segfault_handler(int sig) {
  fprintf(stderr, "\n\n*** SEGMENTATION FAULT DETECTED ***\n");
  fprintf(stderr, "Signal %d received while running test: %s\n", sig,
          current_test);
  fprintf(stderr, "Aborting test...\n\n");
  exit(EXIT_FAILURE);
}

// Create a temporary config file for testing
static void create_test_config() {
  current_test = "create_test_config";
  printf("Creating temporary test config file...\n");

  FILE *config_file = fopen("vibeconfig.json", "w");
  if (!config_file) {
    fprintf(stderr, "ERROR: Could not create config file: %s\n",
            strerror(errno));
    return;
  }

  fprintf(config_file, "{\n"
                       "  \"global\": {\n"
                       "    \"provider\": \"OpenAI\",\n"
                       "    \"api_key\": \"sk-mock-api-key-for-testing\",\n"
                       "    \"default_params\": {\n"
                       "      \"model\": \"gpt-3.5-turbo\",\n"
                       "      \"temperature\": 0.7,\n"
                       "      \"max_tokens\": 150\n"
                       "    }\n"
                       "  }\n"
                       "}\n");
  fclose(config_file);

  // Verify file was created
  if (access("vibeconfig.json", F_OK) != 0) {
    fprintf(stderr, "WARNING: Config file not found after creation attempt\n");
  } else {
    printf("Created temporary test config file successfully\n");
  }
}

// Clean up the temporary config file
static void cleanup_test_config() {
  current_test = "cleanup_test_config";
  printf("Removing temporary test config file...\n");

  if (remove("vibeconfig.json") != 0 && errno != ENOENT) {
    fprintf(stderr, "WARNING: Could not remove config file: %s\n",
            strerror(errno));
  } else {
    printf("Removed temporary test config file successfully\n");
  }
}

// Safe assert macro that won't crash the program
#define SAFE_ASSERT(expr)                                                      \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #expr,       \
              __FILE__, __LINE__);                                             \
      return 0;                                                                \
    }                                                                          \
  } while (0)

// Test the formatting of prompts with variables
static int test_format_prompt() {
  current_test = "test_format_prompt";
  printf("Testing format_prompt()...\n");

  const char *template = "What is the weather like in {city} on {date}?";
  char *var_names[2] = {"city", "date"};
  char *var_values[2] = {"New York", "Monday"};

  char *formatted = format_prompt(template, var_names, var_values, 2);

  SAFE_ASSERT(formatted != NULL);
  printf("  Formatted prompt: %s\n", formatted);

  int result = (strcmp(formatted,
                       "What is the weather like in New York on Monday?") == 0);
  SAFE_ASSERT(result);

  free(formatted);

  printf("format_prompt() test passed!\n");
  return 1;
}

// Test the LLM connection initialization
static int test_llm_connection() {
  current_test = "test_llm_connection";
  printf("Testing LLM connection...\n");

  // Make sure environment variables are set
  printf("  Setting environment variables...\n");
  putenv("VIBELANG_DEV_MODE=1");
  putenv("OPENAI_API_KEY=sk-mock-api-key-for-testing");

  // Verify environment variables
  const char *dev_mode = getenv("VIBELANG_DEV_MODE");
  printf("  VIBELANG_DEV_MODE=%s\n", dev_mode ? dev_mode : "NULL");

  const char *api_key = getenv("OPENAI_API_KEY");
  printf("  OPENAI_API_KEY=%s\n", api_key ? "SET" : "NULL");

  // Initialize the LLM connection
  printf("  Initializing LLM connection...\n");
  int result = init_llm_connection();
  printf("  init_llm_connection() returned: %d\n", result);
  SAFE_ASSERT(result == 1);

  // Clean up
  printf("  Closing LLM connection...\n");
  close_llm_connection();

  printf("LLM connection test passed!\n");
  return 1;
}

// Test sending prompts to the LLM
static int test_send_prompt() {
  current_test = "test_send_prompt";
  printf("Testing send_llm_prompt()...\n");

  // Double-check environment variables
  fprintf(stderr, "TEST_SEND_PROMPT: VIBELANG_DEV_MODE=%s\n",
          getenv("VIBELANG_DEV_MODE") ? getenv("VIBELANG_DEV_MODE") : "NULL");
  fprintf(stderr, "TEST_SEND_PROMPT: OPENAI_API_KEY=%s\n",
          getenv("OPENAI_API_KEY") ? "SET" : "NULL");

  // Initialize the LLM connection
  printf("  Initializing LLM connection...\n");
  int init_result = init_llm_connection();
  printf("  LLM connection result: %d\n", init_result);
  SAFE_ASSERT(init_result == 1);

  // Test with a fixed string to start with
  const char *test_prompt = "What is the weather like in New York?";
  printf("  Sending prompt: '%s'\n", test_prompt);

  // Call with explicit NULL for meaning to verify it works
  char *weather_response = send_llm_prompt(test_prompt, NULL);

  printf("  Response received: %p\n", (void *)weather_response);
  SAFE_ASSERT(weather_response != NULL);

  printf("  Weather response: %s\n", weather_response);
  free(weather_response);

  // Test with a temperature-related prompt
  printf("  Sending temperature prompt...\n");
  char *temp_response = send_llm_prompt("What is the temperature in Paris?",
                                        "temperature in Celsius");
  SAFE_ASSERT(temp_response != NULL);
  printf("  Temperature response: %s\n", temp_response);

  int result = (strcmp(temp_response, "25") == 0);
  SAFE_ASSERT(result);
  free(temp_response);

  // Clean up
  printf("  Closing LLM connection...\n");
  close_llm_connection();

  printf("send_llm_prompt() test passed!\n");
  return 1;
}

// Test the VibeValue creation and access functions
static int test_vibe_values() {
  current_test = "test_vibe_values";
  printf("Testing VibeValue functions...\n");

  // Create a string value
  printf("  Testing string values...\n");
  VibeValue str_val = vibe_string_value("test string");
  SAFE_ASSERT(str_val.type == VIBE_STRING);
  SAFE_ASSERT(str_val.data.string_val != NULL);

  int result = (strcmp(str_val.data.string_val, "test string") == 0);
  SAFE_ASSERT(result);
  free(str_val.data.string_val); // Clean up

  // Create an integer value
  printf("  Testing integer values...\n");
  VibeValue int_val = vibe_int_value(42);
  SAFE_ASSERT(int_val.type == VIBE_NUMBER);
  SAFE_ASSERT(int_val.data.number_val == 42);

  // Create a float value
  printf("  Testing float values...\n");
  VibeValue float_val = vibe_float_value(3.14);
  SAFE_ASSERT(float_val.type == VIBE_NUMBER);
  SAFE_ASSERT(float_val.data.number_val == 3.14);

  // Create a boolean value
  printf("  Testing boolean values...\n");
  VibeValue bool_val = vibe_bool_value(1);
  SAFE_ASSERT(bool_val.type == VIBE_BOOLEAN);
  SAFE_ASSERT(bool_val.data.bool_val == 1);

  printf("VibeValue functions test passed!\n");
  return 1;
}

// Test the runtime execution of prompts
static int test_execute_prompt() {
  current_test = "test_execute_prompt";
  printf("Testing vibe_execute_prompt()...\n");

  // Set environment variables
  printf("  Setting environment variables...\n");
  putenv("VIBELANG_DEV_MODE=1");
  putenv("OPENAI_API_KEY=sk-mock-api-key-for-testing");

  // Initialize the runtime
  printf("  Initializing runtime...\n");
  VibeError err = vibe_runtime_init();
  printf("  vibe_runtime_init() returned: %d\n", err);
  SAFE_ASSERT(err == VIBE_SUCCESS);

  // Test executing a prompt
  printf("  Executing prompt...\n");
  VibeValue result = vibe_execute_prompt("What is the weather like in Tokyo?",
                                         "weather description");

  printf("  Result type: %d\n", result.type);
  SAFE_ASSERT(result.type == VIBE_STRING);
  SAFE_ASSERT(result.data.string_val != NULL);

  printf("  Result value: %s\n", result.data.string_val);
  int test_result = (strstr(result.data.string_val, "Sunny") != NULL);
  SAFE_ASSERT(test_result);

  // Clean up
  printf("  Freeing result...\n");
  if (result.type == VIBE_STRING && result.data.string_val) {
    free(result.data.string_val);
  }

  printf("  Shutting down runtime...\n");
  vibe_runtime_shutdown();

  printf("vibe_execute_prompt() test passed!\n");
  return 1;
}

typedef int (*test_func)(void);

int main() {
  // Install signal handler for debugging segfaults
  signal(SIGSEGV, segfault_handler);

  // Initialize logging with explicit DEBUG level
  init_logging(LOG_LEVEL_DEBUG);
  set_log_level(LOG_LEVEL_DEBUG);

  printf("\n=== Running LLM integration tests ===\n\n");

  // Set up environment variables once globally to ensure they're available
  setenv("VIBELANG_DEV_MODE", "1", 1);
  setenv("OPENAI_API_KEY", "sk-mock-api-key-for-testing", 1);

  // Verify environment variables are set
  fprintf(stderr, "MAIN: VIBELANG_DEV_MODE=%s\n",
          getenv("VIBELANG_DEV_MODE") ? getenv("VIBELANG_DEV_MODE") : "NULL");
  fprintf(stderr, "MAIN: OPENAI_API_KEY=%s\n",
          getenv("OPENAI_API_KEY") ? "SET" : "NULL");

  // Set up test environment
  create_test_config();

  // Define tests to run
  test_func tests[] = {test_format_prompt, test_llm_connection,
                       test_send_prompt, test_vibe_values, test_execute_prompt};
  const char *test_names[] = {"format_prompt", "llm_connection", "send_prompt",
                              "vibe_values", "execute_prompt"};

  // Run each test separately to isolate failures
  int pass_count = 0;
  int total_tests = sizeof(tests) / sizeof(tests[0]);

  for (int i = 0; i < total_tests; i++) {
    printf("\n--- Running test: %s ---\n", test_names[i]);
    fflush(stdout);

    if (tests[i]()) {
      pass_count++;
      printf("--- Test %s PASSED ---\n", test_names[i]);
    } else {
      printf("--- Test %s FAILED ---\n", test_names[i]);
    }
    printf("\n");
  }

  // Clean up test environment
  cleanup_test_config();

  printf("Test summary: %d/%d tests passed\n\n", pass_count, total_tests);

  return (pass_count == total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
