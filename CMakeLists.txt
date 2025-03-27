cmake_minimum_required(VERSION 3.10)
project(vibelang VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add PackCC as an external dependency
include(ExternalProject)
ExternalProject_Add(
    packcc
    GIT_REPOSITORY https://github.com/arithy/packcc.git
    GIT_TAG master
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/packcc
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
)
ExternalProject_Get_Property(packcc source_dir)
set(PACKCC_EXECUTABLE ${source_dir}/packcc)

# Generate parser from grammar
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/parser.c ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/parser.h
    COMMAND ${PACKCC_EXECUTABLE} -o ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/parser ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/grammar.peg
    DEPENDS packcc ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler/grammar.peg
    COMMENT "Generating parser from grammar"
)

# Source files
set(COMPILER_SOURCES
    src/compiler/lexer.c
    src/compiler/parser.c
    src/compiler/ast.c
    src/compiler/semantic.c
    src/compiler/codegen.c
)

set(RUNTIME_SOURCES
    src/runtime/runtime.c
    src/runtime/llm_interface.c
    src/runtime/config.c
)

set(UTILS_SOURCES
    src/utils/file_utils.c
    src/utils/cache_utils.c
    src/utils/log_utils.c
)

# Targets
add_library(vibelang_compiler STATIC ${COMPILER_SOURCES})
add_library(vibelang_runtime STATIC ${RUNTIME_SOURCES})
add_library(vibelang_utils STATIC ${UTILS_SOURCES})

# Main shared library
add_library(vibelang SHARED
    src/vibelang.c
)

target_link_libraries(vibelang
    vibelang_compiler
    vibelang_runtime
    vibelang_utils
    m  # Math library
)

# Compiler tool
add_executable(vibec
    src/tools/vibec.c
)

target_link_libraries(vibec
    vibelang_compiler
    vibelang_utils
)

# Installation
install(TARGETS vibelang vibec
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
)

# Testing
enable_testing()
add_subdirectory(tests)
