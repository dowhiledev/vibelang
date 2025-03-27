# VibeLang Language Guide

This guide covers the syntax and features of the VibeLang programming language.

## Language Overview

VibeLang is a statically-typed language designed for semantic meaning integration and seamless LLM interaction. It compiles to C for efficiency and portability.

## Basic Syntax

### Variable Declaration

Variables are declared using the `let` keyword:

```vibe
let name = "John";
let age = 30;
let isStudent = true;
```

Type annotations are optional but can be specified:

```vibe
let name: String = "John";
let age: Int = 30;
let isStudent: Bool = true;
```

### Types

VibeLang supports the following basic types:

- `Int`: Integer numbers
- `Float`: Floating-point numbers
- `String`: Text strings
- `Bool`: Boolean values (true or false)

### Functions

Functions are defined using the `fn` keyword:

```vibe
fn greet(name: String) -> String {
    return "Hello, " + name + "!";
}
```

Functions can be called as follows:

```vibe
let message = greet("John");
```

### Meaning Types

Meaning types are a unique feature of VibeLang. They attach semantic meaning to basic types:

```vibe
type Temperature = Meaning<Int>("temperature in Celsius");
type Distance = Meaning<Float>("distance in kilometers");
```

Using meaning types:

```vibe
fn convertCelsiusToFahrenheit(temp: Temperature) -> Float {
    return (temp * 9.0 / 5.0) + 32.0;
}
```

### Prompt Blocks

Prompt blocks allow direct integration with LLMs:

```vibe
fn getWeatherInfo(city: String) -> String {
    prompt "What is the current weather like in {city}?"
}
```

Variables from the surrounding scope can be referenced within prompt templates using curly braces.

## Control Flow

### Conditional Statements

```vibe
if (age >= 18) {
    print("Adult");
} else {
    print("Minor");
}
```

### Loops

For loops:

```vibe
for (let i = 0; i < 5; i++) {
    print(i);
}
```

While loops:

```vibe
let i = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}
```

## Classes and Objects

Classes can be defined to create custom data types:

```vibe
class Person {
    name: String;
    age: Int;
    
    fn greet() -> String {
        return "Hello, my name is " + name;
    }
}
```

Creating and using objects:

```vibe
let person = Person();
person.name = "John";
person.age = 30;

print(person.greet());
```

## API Integration

VibeLang can be used from C and Python applications through its API.

### Using From C

```c
#include <vibelang.h>

VibeModule* module = vibelang_load("weather.vibe");
VibeValue* city = vibe_value_string("London");
VibeValue* result = vibe_call(module, "getWeatherInfo", city);

printf("Weather: %s\n", vibe_value_get_string(result));

vibe_value_free(city);
vibe_value_free(result);
vibelang_unload(module);
```

### Using From Python (via bindings)

```python
import vibelang

module = vibelang.load("weather.vibe")
weather = module.call("getWeatherInfo", "London")

print(f"Weather: {weather}")
```

## Advanced Features

### Type Aliases

Simple type aliases can be created:

```vibe
type StringArray = String[];
type IntMap = Map<String, Int>;
```

### Error Handling

VibeLang supports basic error handling:

```vibe
fn divide(a: Int, b: Int) -> Int {
    if (b == 0) {
        error "Cannot divide by zero";
    }
    return a / b;
}
```

## LLM Configuration

VibeLang can be configured to use different LLM providers and settings through the `vibeconfig.json` file.

Example configuration:

```json
{
  "global": {
    "provider": "OpenAI",
    "default_params": {
      "model": "gpt-4",
      "temperature": 0.7
    }
  }
}
```

## Best Practices

- Use meaningful names for variables, functions, and types
- Add semantic meaning to types when appropriate
- Keep prompt templates concise and clear
- Consider caching strategies for expensive LLM calls
- Use appropriate error handling for LLM interactions
