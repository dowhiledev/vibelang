# Top-level Makefile for vibelang

# Default build directory
BUILD_DIR := build

# Default target: build everything
.PHONY: all
all: build

# Create build directory and generate makefiles
.PHONY: configure
configure:
	@mkdir -p $(BUILD_DIR)
	@cmake -B $(BUILD_DIR)

# Build the project
.PHONY: build
build: configure
	@cmake --build $(BUILD_DIR)

# Clean the build directory
.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)

# Run tests
.PHONY: test
test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure

# Run tests with verbose output
.PHONY: test-verbose
test-verbose: build
	@cd $(BUILD_DIR) && ctest --output-on-failure --verbose --rerun-failed 

# Generate parser (Flex/Bison) files
.PHONY: parser
parser: clean-parser prepare-parser-dirs
	@echo "Generating parser using Bison/Flex..."
	@cmake -B $(BUILD_DIR)
	@cmake --build $(BUILD_DIR) --target generate_parser
	@cmake --build $(BUILD_DIR) --target generate_lexer
	@echo "Parser files generated successfully!"

# Prepare directories for parser generation
.PHONY: prepare-parser-dirs
prepare-parser-dirs:
	@mkdir -p $(BUILD_DIR)/bison
	@mkdir -p $(BUILD_DIR)/flex

# Clean up parser generated files
.PHONY: clean-parser
clean-parser:
	@echo "Cleaning up old parser files..."
	@rm -f $(BUILD_DIR)/bison/parser.tab.c $(BUILD_DIR)/bison/parser.tab.h $(BUILD_DIR)/flex/lexer.c $(BUILD_DIR)/flex/lexer.h 2>/dev/null || true
	@echo "Old parser files removed!"

# Install the project
.PHONY: install
install: build
	@cmake --build $(BUILD_DIR) --target install

# Generate documentation
.PHONY: docs
docs:
	@echo "Generating documentation..."
	@cd docs && doxygen Doxyfile 2>/dev/null || echo "Doxygen not found or error occurred."

# Format source code using clang-format
.PHONY: format
format:
	@echo "Formatting source code..."
	@find src include tests -name "*.c" -o -name "*.h" | xargs clang-format -i 2>/dev/null || echo "clang-format not found or error occurred."

# Help target
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all        : Build everything (default target)"
	@echo "  configure  : Configure the build system"
	@echo "  build      : Build the project"
	@echo "  clean      : Clean the build directory"
	@echo "  test       : Run tests"
	@echo "  parser     : Generate parser (Flex/Bison) files"
	@echo "  clean-parser: Clean up parser generated files"
	@echo "  install    : Install the project"
	@echo "  docs       : Generate documentation"
	@echo "  format     : Format source code"
	@echo "  help       : Show this help message"
