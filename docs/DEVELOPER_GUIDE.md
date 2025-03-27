# VibeLang Developer Guide

This document provides detailed information about VibeLang's architecture, implementation, and development workflow for contributors.

## Project Architecture

VibeLang is structured as a multi-layered system:

```ascii
+-------------------+
|   User Programs   |
+-------------------+
          |
   +------v------+
   |  Compiler   |
   +------+------+
          |
  +-------v-------+
  |      C        |
  |  Generation   |
  +-------+-------+
          |
    +-----v------+
    |  Runtime   |
    +-----+------+
          |
  +-------v---------+
  |  LLM Interface  |
  +-----------------+
```

### Core Components

#### 1. Compiler (`src/compiler/`)
- **Lexical Analyzer**: Defined in the grammar.peg file and processed by PackCC
- **Parser**: Generated from grammar.peg by PackCC, builds an AST representation
- **AST**: Abstract Syntax Tree representing the source code structure
- **Semantic Analyzer**: Type checking and symbol resolution
- **Code Generator**: Converts AST into C code

#### 2. Runtime (`src/runtime/`)
- **Module Loader**: Dynamically loads compiled VibeLang modules
- **LLM Interface**: Communicates with language model providers
- **Configuration System**: Manages API keys and LLM settings

#### 3. Utilities (`src/utils/`)
- **AST Utilities**: Functions for working with the AST
- **Logging**: Utilities for diagnostic output
- **File Handling**: File I/O and path manipulation

## Compilation Pipeline

1. **Parsing**: Source code → AST
   - Grammar defined in PEG (Parsing Expression Grammar) format
   - PackCC generates a recursive descent parser

2. **Semantic Analysis**: AST → Validated AST
   - Symbol resolution and type checking
   - Meaning type validation
   - Error detection

3. **Code Generation**: Validated AST → C code
   - Type mapping (VibeLang → C)
   - Prompt block translation
   - Runtime API calls injection

4. **Compilation**: C code → Shared library
   - GCC/Clang compiles generated C code
   - Creates .so/.dll that can be dynamically loaded

## Prompt Handling

When a prompt block is encountered:

1. Variables in scope are identified
2. Template substitution code is generated
3. Runtime call to `vibe_execute_prompt()` is inserted
4. Response conversions to the appropriate return type are added

## Adding a New Feature

### Example: Adding a new language construct

1. Update `grammar.peg` to recognize the new syntax
2. Add corresponding AST node type in `utils/ast.h`
3. Add semantic analysis in `compiler/semantic.c`
4. Add code generation in `compiler/codegen.c`
5. Add tests in `tests/unit/`

### Example: Adding a new LLM provider

1. Update `runtime/config.c` to recognize the new provider
2. Add provider-specific implementation in `runtime/llm_interface.c`
3. Update environment variable handling in both files

## Testing

The project uses a comprehensive test suite:

- **Unit Tests**: Test individual components in isolation
- **Integration Tests**: Test the full compilation pipeline
- **Generated Code Tests**: Verify the correctness of generated C code

To run tests:

```bash
cd build
make test
```

## Build System

The project uses CMake as its build system:

- CMake configuration in `CMakeLists.txt`
- Dependencies managed through CMake's find_package
- PackCC integration for parser generation

## Code Style

- **C Code**: 4-space indentation, C11 standard
- **Comments**: Function documentation in header files
- **Naming**: Snake case for functions and variables

## Common Pitfalls

- **Memory Management**: Ensure proper cleanup in all code paths
- **Error Handling**: Check return values and propagate errors
- **Grammar Ambiguities**: Be careful when updating the grammar
- **API Version Compatibility**: Maintain backward compatibility

## References

- [PackCC Documentation](https://github.com/arithy/packcc)
- [OpenAI API Documentation](https://platform.openai.com/docs/api-reference)
- [C11 Standard](https://en.cppreference.com/w/c/11)
