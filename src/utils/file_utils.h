#ifndef VIBELANG_FILE_UTILS_H
#define VIBELANG_FILE_UTILS_H

#include <stdio.h>

/* Read entire file into memory */
char* read_file(const char* filename);

/* Get directory path from a file path */
char* get_directory_path(const char* filepath);

/* Join two paths */
char* path_join(const char* path1, const char* path2);

/* Check if file exists */
int file_exists(const char* filename);

/* Get file modification time */
long long get_file_mtime(const char* filename);

/* Get file extension */
const char* get_file_extension(const char* filename);

/* Create directories recursively */
int create_directories(const char* path);

#endif /* VIBELANG_FILE_UTILS_H */
