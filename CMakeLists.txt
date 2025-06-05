cmake_minimum_required(VERSION 3.10)
project(vibelang VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Enable testing
enable_testing()
include(CTest)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set RPATH settings for proper runtime library resolution
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
if(APPLE)
  # On macOS, also add the executable directory to RPATH
  set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/../lib;${CMAKE_INSTALL_RPATH}")
  # Ensure executables look in their directory for dynamic libraries
  set(CMAKE_INSTALL_NAME_DIR "@rpath")
  set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR TRUE)
endif()

# Find required packages
find_package(CURL REQUIRED)
if(CURL_FOUND)
  include_directories(${CURL_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "Could not find libcurl. Please install libcurl development package.")
endif()

# Flex and Bison output directories
set(FLEX_OUTPUT_DIR ${CMAKE_BINARY_DIR}/flex)
set(BISON_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bison)
file(MAKE_DIRECTORY ${FLEX_OUTPUT_DIR} ${BISON_OUTPUT_DIR})

# Mark parser outputs as generated to avoid configure-time checks
set_source_files_properties(
    ${BISON_OUTPUT_DIR}/parser.tab.c ${BISON_OUTPUT_DIR}/parser.tab.h
    ${FLEX_OUTPUT_DIR}/lexer.c ${FLEX_OUTPUT_DIR}/lexer.h
    PROPERTIES GENERATED TRUE
)

# Include directories
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cjson  
  ${FLEX_OUTPUT_DIR}
  ${BISON_OUTPUT_DIR}
)

# Check for system cJSON or use our own
find_library(CJSON_LIBRARY NAMES cjson)
find_path(CJSON_INCLUDE_DIR NAMES cJSON.h)

if(CJSON_LIBRARY AND CJSON_INCLUDE_DIR)
  message(STATUS "Using system cJSON: ${CJSON_LIBRARY}")
  set(CJSON_LIBRARIES ${CJSON_LIBRARY})
  include_directories(${CJSON_INCLUDE_DIR})
else()
  message(STATUS "Using bundled cJSON")
  # Add our own cJSON implementation
  add_library(cjson STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cjson/cJSON.c
  )
  set(CJSON_LIBRARIES cjson)
  # Include directory already added above
endif()

# Add internal libraries
add_library(vibelang_utils STATIC
  src/utils/ast.c
  src/utils/log_utils.c
  src/utils/file_utils.c
  src/utils/cache_utils.c
)
set_target_properties(vibelang_utils PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(vibelang_runtime STATIC
  src/runtime/config.c
  src/runtime/runtime.c
  src/runtime/llm_interface.c
)
set_target_properties(vibelang_runtime PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Add compiler library
add_library(vibelang_compiler STATIC
  src/compiler/parser_utils.c
  src/compiler/parser_impl.c
  ${BISON_OUTPUT_DIR}/parser.tab.c
  ${FLEX_OUTPUT_DIR}/lexer.c
  src/compiler/symbol_table.c
  src/compiler/semantic.c
  src/compiler/codegen.c
)
set_target_properties(vibelang_compiler PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Include the compiler-specific CMake configuration
add_subdirectory(src/compiler)

# Add main library
add_library(vibelang SHARED
  src/vibelang.c
)

# Add dependencies
target_link_libraries(vibelang_utils PRIVATE ${CJSON_LIBRARIES})
target_link_libraries(vibelang_runtime PRIVATE vibelang_utils ${CJSON_LIBRARIES} ${CURL_LIBRARIES})
target_link_libraries(vibelang_compiler PRIVATE vibelang_utils)
target_link_libraries(vibelang PUBLIC 
  vibelang_utils 
  vibelang_runtime 
  vibelang_compiler 
  ${CJSON_LIBRARIES}
  ${CURL_LIBRARIES}
)

# Add vibec executable
add_executable(vibec
  src/tools/vibec.c
)

# IMPORTANT: Link vibec AFTER defining the executable
target_link_libraries(vibec PUBLIC vibelang)
target_link_libraries(vibec PRIVATE vibelang_compiler)

# On macOS, set the proper RPATH for the executable
if(APPLE)
  set_target_properties(vibec PROPERTIES
    INSTALL_RPATH "@executable_path;@executable_path/../lib"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
  # Ensure proper linking of static libraries
  set_target_properties(vibelang PROPERTIES 
    LINK_FLAGS "-Wl,-all_load"
    MACOSX_RPATH TRUE
    INSTALL_NAME_DIR "@rpath"
  )
endif()

# Add test subdirectory
add_subdirectory(tests)

# Installation targets
install(TARGETS vibelang vibec
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(FILES include/vibelang.h include/runtime.h src/utils/ast.h
  DESTINATION include
)
