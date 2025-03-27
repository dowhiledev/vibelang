cmake_minimum_required(VERSION 3.10)
project(vibelang VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# macOS specific settings for SDK path and deployment target
if(APPLE)
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # Remove any leading/trailing whitespace that might cause issues
    string(STRIP "${MACOS_SDK_PATH}" MACOS_SDK_PATH)
    set(CMAKE_OSX_SYSROOT "${MACOS_SDK_PATH}")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version")
endif()

# Find required packages
find_package(CURL REQUIRED)
find_package(cJSON REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${CJSON_INCLUDE_DIRS})

# Add PackCC as an external dependency, with direct compiler command
include(ExternalProject)
ExternalProject_Add(
    packcc
    GIT_REPOSITORY https://github.com/arithy/packcc.git
    GIT_TAG main
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/packcc
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${CMAKE_C_COMPILER} -isysroot "${MACOS_SDK_PATH}" -o <BINARY_DIR>/packcc <SOURCE_DIR>/src/packcc.c
    BUILD_IN_SOURCE 0
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS <BINARY_DIR>/packcc
)
ExternalProject_Get_Property(packcc BINARY_DIR)
set(PACKCC_EXECUTABLE ${BINARY_DIR}/packcc)

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
    src/compiler/parser_utils.c
    src/compiler/semantic.c
    src/compiler/codegen.c
    src/compiler/symbol_table.c
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
    src/utils/ast.c
)

# Targets
add_library(vibelang_compiler STATIC ${COMPILER_SOURCES})
add_library(vibelang_runtime STATIC ${RUNTIME_SOURCES})
add_library(vibelang_utils STATIC ${UTILS_SOURCES})

target_link_libraries(vibelang_runtime 
    ${CURL_LIBRARIES}
    ${CJSON_LIBRARIES}
    dl  # For dynamic loading
)

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
