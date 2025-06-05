# Compiler-specific CMake configuration

# Find Flex and Bison
find_package(BISON)
find_package(FLEX)

# Set output directories
set(BISON_OUTPUT_DIR ${CMAKE_BINARY_DIR}/bison)
set(FLEX_OUTPUT_DIR ${CMAKE_BINARY_DIR}/flex)

# Ensure directories exist
file(MAKE_DIRECTORY ${BISON_OUTPUT_DIR})
file(MAKE_DIRECTORY ${FLEX_OUTPUT_DIR})

# Define the parser source files
set(BISON_PARSER_SOURCE ${BISON_OUTPUT_DIR}/parser.tab.c)
set(BISON_PARSER_HEADER ${BISON_OUTPUT_DIR}/parser.tab.h)
set(FLEX_LEXER_SOURCE ${FLEX_OUTPUT_DIR}/lexer.c)
set(FLEX_LEXER_HEADER ${FLEX_OUTPUT_DIR}/lexer.h)

# Mark these files as generated so CMake doesn't require them at configure time
set_source_files_properties(
    ${BISON_PARSER_SOURCE} ${BISON_PARSER_HEADER}
    ${FLEX_LEXER_SOURCE} ${FLEX_LEXER_HEADER}
    PROPERTIES GENERATED TRUE
)

# Generate parser with Bison
if(BISON_FOUND)
    add_custom_command(
        OUTPUT ${BISON_PARSER_SOURCE} ${BISON_PARSER_HEADER}
        COMMAND ${BISON_EXECUTABLE}
        ARGS --defines=${BISON_PARSER_HEADER}
             -o ${BISON_PARSER_SOURCE}
             ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
        COMMENT "Generating parser with Bison"
        VERBATIM
    )
else()
    message(WARNING "Bison not found - parser will not be generated!")
endif()

# Generate lexer with Flex
if(FLEX_FOUND)
    add_custom_command(
        OUTPUT ${FLEX_LEXER_SOURCE}
        COMMAND ${FLEX_EXECUTABLE}
        ARGS -o ${FLEX_LEXER_SOURCE}
             ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l ${BISON_PARSER_HEADER}
        COMMENT "Generating lexer with Flex"
        VERBATIM
    )
else()
    message(WARNING "Flex not found - lexer will not be generated!")
endif()

# Create custom targets for the generated files
add_custom_target(generate_parser
    DEPENDS ${BISON_PARSER_SOURCE} ${BISON_PARSER_HEADER}
    COMMENT "Generated Bison parser files"
)

add_custom_target(generate_lexer
    DEPENDS ${FLEX_LEXER_SOURCE}
    COMMENT "Generated Flex lexer files"
)

# Make the lexer depend on the parser to ensure correct build order
add_dependencies(generate_lexer generate_parser)

# Add the parser implementation files to the compiler sources
# Use the full path to avoid relative path issues
target_sources(vibelang_compiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/parser_impl.c
)

# The vibelang_compiler target is defined in the parent directory. Ensure it
# depends on the generated parser and lexer so they are built automatically.
add_dependencies(vibelang_compiler generate_parser generate_lexer)

# Include the generated headers in the compiler's include path
target_include_directories(vibelang_compiler 
    PRIVATE
    ${CMAKE_BINARY_DIR}/bison
    ${CMAKE_BINARY_DIR}/flex
)
