/**
 * @file parser_internal.h
 * @brief Internal definitions for the Vibe language parser
 */

#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H

#include "../utils/ast.h"

/**
 * Structure representing a Vibe parser context
 */
struct vibe_context_tag {
    void* auxil;            // Auxiliary data (often the source string)
    size_t pos;             // Current position in the input
    char* buffer;           // Working buffer for the parser
    size_t buffer_capacity;  // Capacity of the buffer
    size_t buffer_used;     // Amount of buffer currently in use
    int depth;              // Current parsing depth (for nested structures)
    ast_node_t* result;     // Result of parsing
};

// Helper function used by the parser to extract text
const char* text(const void* auxil);

#endif /* PARSER_INTERNAL_H */
