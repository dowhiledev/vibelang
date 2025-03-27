# Unit Tests for VibeLang

# Create the test data directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/unit/data)

add_executable(test_ast
    unit/test_ast.c
)
target_link_libraries(test_ast
    vibelang_utils
)
add_test(NAME test_ast COMMAND test_ast)

add_executable(test_parser
    unit/test_parser.c
)
target_link_libraries(test_parser
    vibelang_compiler
    vibelang_utils
)
add_test(NAME test_parser COMMAND test_parser)

add_executable(test_semantic
    unit/test_semantic.c
)
target_link_libraries(test_semantic
    vibelang_compiler
    vibelang_utils
)
add_test(NAME test_semantic COMMAND test_semantic)

add_executable(test_codegen
    unit/test_codegen.c
)
target_link_libraries(test_codegen
    vibelang_compiler
    vibelang_utils
)
add_test(NAME test_codegen COMMAND test_codegen)

add_executable(test_parser_utils
    unit/test_parser_utils.c
)
target_link_libraries(test_parser_utils
    vibelang_compiler
    vibelang_utils
)
add_test(NAME test_parser_utils COMMAND test_parser_utils)
