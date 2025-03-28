# VibeLang

VibeLang is a programming language designed to seamlessly integrate with Large Language Models (LLMs) through native prompt blocks.

## Features

- **Semantic Types** - Add meaning to your data types
- **Native LLM Integration** - Use prompt blocks directly in your code
- **Strong Type System** - Static typing with inference
- **C Interoperability** - Compiles to C for portability and performance

## Getting Started

### Prerequisites

- CMake 3.10+
- C compiler (GCC, Clang)
- libcURL development libraries
- (Optional) cJSON development libraries

### Installation

#### Building from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/vibelang.git
cd vibelang

# Build the project
mkdir -p build && cd build
cmake ..
make

# Install globally (optional)
sudo make install
```

For macOS users, you can also use the provided installation script:

```bash
# From the root of the repository
chmod +x install.sh
./install.sh
```

This script properly handles dynamic library paths for macOS.

### Your First VibeLanguage Program

Create a file called `greeting.vibe`:

```vibe
type Greeting = Meaning<String>("greeting message");

fn greet(name: String) -> Greeting {
    prompt "Generate a friendly greeting for a person named {name} starting with Hello";
}

fn main() {
    let message = greet("World");
    print(message);
}
```

Compile and run:

```bash
vibec greeting.vibe
gcc greeting.c -o greeting -lvibelang
./greeting
```

## LLM Configuration

Configure your LLM provider by creating a `vibeconfig.json` file:

```json
{
  "global": {
    "provider": "OpenAI",
    "api_key": "your-api-key-here",
    "default_params": {
      "model": "gpt-4",
      "temperature": 0.7
    }
  }
}
```

You can also set the `VIBE_API_KEY` environment variable instead of including it in the config file.

## Documentation

- [Language Guide](docs/LANGUAGE_GUIDE.md) - Learn the VibeLang syntax and features
- [API Reference](docs/API_REFERENCE.md) - Detailed API documentation
- [Developer Guide](docs/DEVELOPER_GUIDE.md) - Contributing to VibeLang
- [Implementation Details](docs/IMPLEMENTATION.md) - Technical details about the compiler

## License

VibeLang is licensed under the MIT License. See [LICENSE](LICENSE) for details.
