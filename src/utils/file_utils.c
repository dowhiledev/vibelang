#include "file_utils.h"
#include "log_utils.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Read entire file into memory */
char *read_file(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    ERROR("Failed to open file '%s': %s", filename, strerror(errno));
    return NULL;
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);

  // Allocate buffer
  char *buffer = (char *)malloc(size + 1);
  if (!buffer) {
    ERROR("Memory allocation failed for file '%s'", filename);
    fclose(file);
    return NULL;
  }

  // Read file content
  size_t read_size = fread(buffer, 1, size, file);

  if (read_size != (size_t)size) {
    ERROR("Failed to read file '%s': %s", filename, strerror(errno));
    free(buffer);
    fclose(file);
    return NULL;
  }

  buffer[size] = '\0'; // Null-terminate the string
  fclose(file);

  return buffer;
}

/* Get directory path from a file path */
char *get_directory_path(const char *filepath) {
  if (!filepath)
    return NULL;

  char *path = strdup(filepath);
  if (!path)
    return NULL;

  char *last_slash = strrchr(path, '/');

  if (last_slash) {
    *last_slash = '\0';
    return path;
  } else {
    free(path);
    return strdup("."); // Current directory
  }
}

/* Join two paths */
char *path_join(const char *path1, const char *path2) {
  if (!path1 || !path2)
    return NULL;

  // Check if path2 is absolute
  if (path2[0] == '/') {
    return strdup(path2);
  }

  size_t len1 = strlen(path1);
  size_t len2 = strlen(path2);

  // Calculate the size needed
  size_t size = len1 + 1 + len2 + 1; // path1 + '/' + path2 + '\0'

  // Allocate memory
  char *result = (char *)malloc(size);
  if (!result)
    return NULL;

  // Copy path1
  strcpy(result, path1);

  // Add separator if needed
  if (len1 > 0 && path1[len1 - 1] != '/') {
    strcat(result, "/");
  }

  // Add path2
  strcat(result, path2);

  return result;
}

/* Check if file exists */
int file_exists(const char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

/* Get file modification time */
long long get_file_mtime(const char *filename) {
  struct stat buffer;

  if (stat(filename, &buffer) == 0) {
    return (long long)buffer.st_mtime;
  }

  return -1; // Error
}

/* Get file extension */
const char *get_file_extension(const char *filename) {
  if (!filename)
    return NULL;

  const char *dot = strrchr(filename, '.');

  if (!dot || dot == filename) {
    return "";
  }

  return dot + 1;
}

/* Create directories recursively */
int create_directories(const char *path) {
  if (!path)
    return 0;

  char *temp_path = strdup(path);
  if (!temp_path)
    return 0;

  char *p = temp_path;

  // Skip leading slashes
  if (*p == '/') {
    p++;
  }

  while (*p) {
    // Find the next slash
    char *slash = strchr(p, '/');
    if (slash) {
      *slash = '\0'; // Temporarily terminate the string
    }

    // Create directory
    struct stat st;
    if (stat(temp_path, &st) != 0) {
#ifdef _WIN32
      int result = mkdir(temp_path);
#else
      int result = mkdir(temp_path, 0755);
#endif
      if (result != 0 && errno != EEXIST) {
        ERROR("Failed to create directory '%s': %s", temp_path,
              strerror(errno));
        free(temp_path);
        return 0;
      }
    }

    // Restore the slash
    if (slash) {
      *slash = '/';
      p = slash + 1;
    } else {
      break;
    }
  }

  free(temp_path);
  return 1;
}
