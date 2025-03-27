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
