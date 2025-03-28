# Contributing to VibeLang

Thank you for your interest in contributing to VibeLang! This document provides guidelines and instructions for contributing to the project.

## Getting Started

### Prerequisites

- C/C++ development environment
- CMake 3.10+
- Git
- libcurl and cJSON development libraries

### Setting Up the Development Environment

1. Fork the repository on GitHub
2. Clone your fork locally:
   ```bash
   git clone https://github.com/yourusername/vibelang.git
   cd vibelang
   ```
3. Set up the build environment:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```
4. Run the tests to verify your setup:
   ```bash
   make test
   ```

## Development Workflow

### Branching Strategy

- `main`: Stable release branch
- `develop`: Development branch for integrating features
- Feature branches: For specific features or fixes

When working on a new feature or fix:

1. Create a branch from `develop`:
   ```bash
   git checkout develop
   git checkout -b feature/your-feature-name
   ```

2. Implement your changes with appropriate tests

3. Open a pull request against the `develop` branch

### Code Style

We follow these general guidelines:

- C code: 4-space indentation, C11 standard
- Use clear, descriptive names for functions and variables
- Document public functions with comments
- Add tests for new functionality

### Commit Messages

Please use clear and descriptive commit messages:

- Begin with a short summary line (50 chars or less)
- Optionally add a blank line and detailed explanation
- Use the present tense ("Add feature" not "Added feature")

Example:
```bash
Add semantic analysis for meaning types

This commit adds support for semantic analysis of meaning types in the
compiler. It includes validation for type compatibility and error handling
for invalid declarations.
```

## Documentation

Good documentation is essential. Please follow these guidelines:

1. **Code Documentation**: All public functions, types, and constants should be documented using Doxygen-style comments:

```c
/**
 * @brief Brief description
 * @param paramName Description of parameter
 * @return Description of return value
 */
```

2. **README and Guides**: Keep the README, API reference, and developer guides updated when adding new features.

3. **Generate Documentation**: Run `make docs` to verify that your documentation can be generated correctly.

## Testing

### Unit Tests

Unit tests should be added for all new functionality. Tests are located in the `tests/unit` directory.

To add a new test:

1. Create a new test file in `tests/unit`.
2. Add the test to `tests/CMakeLists.txt`.
3. Use assertions to verify expected behavior.

### Integration Tests

Integration tests verify the complete pipeline. Add integration tests to the `tests/integration` directory.

### Running Tests

To run the tests:

```bash
cd build
make test
```

### Debugging Tests

Use the `test-verbose` target for detailed output:

```bash
make test-verbose
```

## Submitting Changes

1. Ensure your code passes all tests.
2. Update documentation as needed.
3. Open a pull request with a clear description of your changes.

Thank you for contributing to VibeLang!