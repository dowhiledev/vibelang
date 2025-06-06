# Tests for VibeLanguage

# Enable testing
enable_testing()

# Set test macros
include(CTest)

# Include directories
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}/../src/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/cjson
  ${CMAKE_BINARY_DIR}/flex
  ${CMAKE_BINARY_DIR}/bison
)

# Create test for AST functions
add_executable(test_ast 
  unit/test_ast.c
)
target_link_libraries(test_ast PRIVATE vibelang_utils cjson m)
add_test(NAME test_ast COMMAND test_ast)

# Create test for parser utilities
add_executable(test_parser_utils 
  unit/test_parser_utils.c
)
target_link_libraries(test_parser_utils PRIVATE vibelang_compiler vibelang_utils cjson)
add_test(NAME test_parser_utils COMMAND test_parser_utils)

# Create test for Bison parser
add_executable(test_bison_parser 
  unit/test_bison_parser.c
)
target_link_libraries(test_bison_parser PRIVATE vibelang_compiler vibelang_utils cjson)
add_test(NAME test_bison_parser COMMAND test_bison_parser)

# Create test for semantic analysis
add_executable(test_semantic 
  unit/test_semantic.c
)
target_link_libraries(test_semantic PRIVATE vibelang_compiler vibelang_utils cjson)
add_test(NAME test_semantic COMMAND test_semantic)

# Create test for code generation
add_executable(test_codegen 
  unit/test_codegen.c
)
target_link_libraries(test_codegen PRIVATE vibelang_compiler vibelang_utils cjson)
add_test(NAME test_codegen COMMAND test_codegen)

# Add unit test executable for the LLM integration
add_executable(test_llm_integration 
  unit/test_llm_integration.c
)

# Link against the library
target_link_libraries(test_llm_integration 
  vibelang_runtime
  vibelang_utils
)

# Add to CTest test suite
add_test(NAME test_llm_integration
         COMMAND test_llm_integration)

# Test that vibec embeds the correct rpath in generated shared libraries
add_executable(test_vibec_rpath
  unit/test_vibec_rpath.c
)
add_test(NAME test_vibec_rpath COMMAND test_vibec_rpath)

# Create test data directory during build
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/unit/data)

# Copy test data files if they exist
file(GLOB TEST_DATA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/unit/data/*")
foreach(TEST_DATA_FILE ${TEST_DATA_FILES})
  get_filename_component(TEST_DATA_FILE_NAME ${TEST_DATA_FILE} NAME)
  configure_file(${TEST_DATA_FILE} ${CMAKE_BINARY_DIR}/tests/unit/data/${TEST_DATA_FILE_NAME} COPYONLY)
endforeach()
