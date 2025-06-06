#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

static int run_cmd(const char *cmd) {
  int ret = system(cmd);
  if (ret != 0) {
    fprintf(stderr, "Command failed: %s\n", cmd);
  }
  return ret;
}

static int test_vibec_rpath(void) {
  printf("Running vibec rpath test...\n");

  /* Determine install prefix (build directory parent) */
  char prefix[PATH_MAX];
  if (!realpath("..", prefix)) {
    perror("realpath");
    return 1;
  }

  char env_var[PATH_MAX + 10];
  snprintf(env_var, sizeof(env_var), "PREFIX=%s", prefix);
  putenv(env_var);

  /* Create a simple vibe source */
  const char *src = "tests/unit/data/rpath_test.vibe";
  FILE *f = fopen(src, "w");
  if (!f) {
    perror("fopen");
    return 1;
  }
  fputs("type Joke = Meaning<String>(\"a short humorous line\");\n", f);
  fputs("fn tellJoke() -> Joke {\n", f);
  fputs("    prompt \"Tell me a short joke.\";\n", f);
  fputs("}\n", f);
  fclose(f);

  /* Compile with vibec */
  char cmd[PATH_MAX * 2];
  snprintf(cmd, sizeof(cmd), "../bin/vibec %s > /dev/null", src);
  if (run_cmd(cmd) != 0) {
    return 1;
  }

  /* Check generated shared library for runpath */
  const char *lib = "tests/unit/data/rpath_test.so";
#ifdef __APPLE__
  snprintf(cmd, sizeof(cmd),
           "otool -l %s | grep -A2 RPATH | grep %s/lib > /dev/null", lib, prefix);
#else
  snprintf(cmd, sizeof(cmd),
           "readelf -d %s | grep -E '(RUNPATH|RPATH).*%s/lib' > /dev/null", lib,
           prefix);
#endif
  if (run_cmd(cmd) != 0) {
    printf("rpath not found\n");
    return 1;
  }

  /* Cleanup */
  unlink("tests/unit/data/rpath_test.c");
  unlink("tests/unit/data/rpath_test.so");
  unlink(src);

  printf("vibec rpath test passed!\n");
  return 0;
}

int main(void) { return test_vibec_rpath(); }
