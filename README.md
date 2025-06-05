<h1>
  <div>
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="assets/icon-light.svg" width="400" height="100">
      <source media="(prefers-color-scheme: light)" srcset="assets/icon-dark.svg" width="60" height="60">
      <img alt="VibeLang Icon" style="vertical-align: middle;">
    </picture>
    <!-- <span style="vertical-align: middle; margin-left: 10px;">VibeLang</span> -->
  </div>
</h1>

<div >

[![Tests](https://github.com/dowhiledev/vibelang/actions/workflows/tests.yml/badge.svg)](https://github.com/dowhiledev/vibelang/actions/workflows/tests.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-0.1.0-brightgreen.svg)](https://github.com/dowhiledev/vibelang)

</div>

VibeLang is a statically-typed language designed for natural language interactions with LLM models. It provides a type-safe way to integrate AI capabilities into applications.

## Project Status

- ✅ Milestone 1: Core Compiler Development
- ✅ Milestone 2: Code Generation & C API Exposure
- ✅ Milestone 3: Runtime Library & LLM Integration
- 🔄 Milestone 4: Cross-Language Support and Wrappers
- 🔄 Milestone 5: Comprehensive Testing and Documentation

## Documentation

The API reference and developer documentation are available [Here](https://dowhiledev.github.io/vibelang/).

You can also generate the documentation locally using:
```bash
make docs
```

And view it by running:
```bash
make serve-docs
```

## Features

- Statically typed language with semantic meaning support
- Direct integration with LLM providers
- Natural language prompt templates
- Type-safe API for C and Python

## Runtime Library

The VibeLang runtime provides:

1. **LLM Connection**: Connect to various LLM providers (currently supports OpenAI)
2. **Prompt Execution**: Send prompts to LLMs with automatic variable substitution
3. **Module Loading**: Dynamically load and execute compiled VibeLang modules

### Configuration

LLM settings can be configured in a `vibeconfig.json` file:

```json
{
  "global": {
    "provider": "OpenAI",
    "api_key": "YOUR_API_KEY_HERE", 
    "default_params": {
      "model": "gpt-3.5-turbo",
      "temperature": 0.7,
      "max_tokens": 150
    }
  }
}
```

API keys can also be provided via environment variables:
- `OPENAI_API_KEY` - For OpenAI API access
- `VIBELANG_API_KEY` - Generic API key for any provider

## Getting Started

### Prerequisites

- C compiler (GCC or Clang)
- CMake 3.10+
- libcurl and cJSON development libraries
- Bison and Flex (for building the parser)

### Installation

```bash
git clone https://github.com/dowhiledev/vibelang.git
cd vibelang
mkdir build && cd build
cmake ..
make
sudo make install
```

### Writing Your First VibeLang Program

Create a file named `weather.vibe`:

```
type Temperature = Meaning<Int>("temperature in Celsius");
type Forecast = Meaning<String>("weather forecast description");

fn getTemperature(city: String) -> Temperature {
    prompt "What is the current temperature in {city} in Celsius?";
}

fn getForecast(city: String, day: String) -> Forecast {
    prompt "Provide a brief weather forecast for {city} on {day}.";
}
```

Compile and use it:

```bash
vibec weather.vibe
gcc -o weather_app weather_app.c -lvibelang
./weather_app
```

## Known Issues

- Function overloading is not yet supported
- Limited error reporting from LLM providers
- No streaming support for LLM responses yet

## VS Code Extension

A minimal VS Code extension is included in the `vscode` directory. It provides
syntax highlighting for `.vibe` files. To try it out:

1. Open the `vscode` folder in VS Code.
2. Press `F5` to launch an Extension Development Host.
3. Open a VibeLang source file to see the highlighting.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
