# VibeLang

VibeLang is a high-level programming language designed for seamless integration with Large Language Models (LLMs). It features semantic type annotations, built-in prompt blocks, and interoperability with C and Python.

## Features

- **Semantic Type System** - Attach meaning to your data types for better LLM understanding
- **Built-in LLM Integration** - First-class support for LLM prompts within code using `prompt` blocks
- **Meaning Annotations** - Allow LLMs to understand the purpose and context of variables
- **Cross-language Compatibility** - Call VibeLang functions from C and Python
- **Efficient Compilation** - Compiles to C for performance and portability

## How It Works

VibeLang compiles to C code and exposes a simple API for integration with other languages. The compiler:

1. Parses VibeLang source files using a PEG-based grammar
2. Performs semantic analysis to ensure type safety
3. Generates C code that includes calls to the VibeLang runtime
4. Compiles the generated code into a shared library
5. Provides runtime LLM integration through configurable providers (e.g., OpenAI)

When a `prompt` block runs, VibeLang:

1. Formats the prompt template with variable values
2. Sends the prompt to the configured LLM provider
3. Parses the response and converts it to the appropriate return type
4. Caches responses for efficiency (configurable)

## Getting Started

### Prerequisites

- C compiler (GCC or Clang)
- CMake 3.10+
- libcurl and cJSON libraries
- Python 3.6+ (for Python bindings)

### Installing Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get install build-essential cmake libcurl4-openssl-dev libcjson-dev
```

#### macOS
```bash
brew install cmake curl cjson
```

### Building from Source

```bash
git clone https://github.com/yourusername/vibelang.git
cd vibelang
mkdir build && cd build
cmake ..
make
sudo make install
```

### Setting up the LLM provider

Set your API key in the environment:
```bash
export OPENAI_API_KEY=your-api-key
```

Or in `vibeconfig.json` in your project directory:
```json
{
  "global": {
    "provider": "OpenAI",
    "api_key": "your-api-key",
    "default_params": { 
      "model": "gpt-4",
      "temperature": 0.7, 
      "max_tokens": 150 
    }
  }
}
```

## Basic Example

### Temperature Checker

```vibe
type Temperature = Meaning<Int>("temperature in Celsius");

fn getTemperature(city: Meaning<String>("city name")) -> Temperature {
    prompt "What is the temperature in {city}?"
}

fn main() {
    let temp = getTemperature("San Francisco");
    print("The temperature is {temp} degrees Celsius");
}
```

### Multilingual Greeting Generator

```vibe
type Greeting = Meaning<String>("a friendly greeting");
type Language = Meaning<String>("language name");

fn generateGreeting(name: Meaning<String>("person's name"), 
                   language: Language) -> Greeting {
    prompt "Generate a friendly greeting for a person named {name} in {language}."
}

fn main() {
    let languages = ["English", "Spanish", "French", "Japanese"];
    
    for (let i = 0; i < 4; i++) {
        let greeting = generateGreeting("Alex", languages[i]);
        print("{greeting}");
    }
}
```

## Using in C Programs

```c
#include <vibelang.h>
#include <stdio.h>

int main() {
    // Initialize the VibeLang runtime
    vibelang_init();
    
    // Load a VibeLang module
    VibeModule* weather = vibelang_load("weather.vibe");
    
    // Create argument
    VibeValue* city = vibe_value_string("Tokyo");
    
    // Call a function
    VibeValue* temp = vibe_call(weather, "getTemperature", city);
    
    // Use the result
    printf("Temperature: %lld°C\n", vibe_value_get_int(temp));
    
    // Clean up
    vibe_value_free(city);
    vibe_value_free(temp);
    vibelang_unload(weather);
    vibelang_shutdown();
    
    return 0;
}
```

## Configuration Options

VibeLang uses a `vibeconfig.json` file for configuration:

```json
{
  "global": {
    "provider": "OpenAI",
    "api_key": "your-api-key",
    "default_params": { 
      "model": "gpt-4",
      "temperature": 0.7, 
      "max_tokens": 150 
    }
  },
  "overrides": {
    "getTemperature": { 
      "temperature": 0.5,
      "max_tokens": 100
    },
    "greet": { 
      "temperature": 0.9
    }
  }
}
```

## Project Structure

- `include/` - Public C API headers
- `src/` - Source code for compiler and runtime
  - `compiler/` - Parsing, semantic analysis, and code generation
  - `runtime/` - Module loading and LLM integration
  - `utils/` - Utilities for AST, file handling, etc.
- `bindings/` - Language bindings (Python)
- `examples/` - Example VibeLang programs
- `tests/` - Unit and integration tests

## Development

### Running the Compiler

```bash
vibec -o output.c input.vibe  # Compile only
vibec input.vibe              # Compile and generate shared library
```

### Debugging

Use the `-v` flag for verbose output:

```bash
vibec -v input.vibe
```

## Contributing

Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for details.

## License

[MIT License](LICENSE)
