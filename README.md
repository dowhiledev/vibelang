# VibeLang

VibeLang is a high-level programming language designed for easy integration with Large Language Models (LLMs). It features semantic type annotations, built-in prompt blocks, and seamless interoperability with C and Python.

## Features

- **Semantic Type System** - Attach meaning to your data types
- **Built-in LLM Integration** - First-class support for LLM prompts within code
- **Cross-language Compatibility** - Call VibeLang functions from C and Python
- **Efficient Compilation** - Compiles to C for performance and portability

## Getting Started

### Prerequisites

- C compiler (GCC or Clang)
- CMake 3.10+
- Python 3.6+ (for Python bindings)

### Building from Source

```bash
mkdir build && cd build
cmake ..
make
make install
```

### Basic Example

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

## Configuration

VibeLang uses a `vibeconfig.json` file to configure LLM providers and parameters:

```json
{
  "global": {
    "provider": "OpenAI",
    "api_key": "your-api-key",
    "default_params": { "temperature": 0.7, "max_tokens": 150 }
  }
}
```

## Project Structure

- `include/` - Public C API headers
- `src/` - Source code for compiler and runtime
- `bindings/` - Language bindings (Python)
- `examples/` - Example VibeLang programs
- `tests/` - Unit and integration tests

## License

[MIT License](LICENSE)
