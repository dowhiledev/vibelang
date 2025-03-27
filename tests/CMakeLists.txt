# Unit Tests for VibeLang

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
