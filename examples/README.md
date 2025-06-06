# VibeLang Examples

This directory contains sample `.vibe` programs that can be compiled with `vibec`.

## Running the Joke Example

1. Ensure `OPENAI_API_KEY` is set in your environment. Example:
   ```bash
   export OPENAI_API_KEY=YOUR_OPENAI_API_KEY
   ```
   Replace `YOUR_OPENAI_API_KEY` with the actual key provided by OpenAI.

2. Compile the example and run:
   ```bash
vibec joke.vibe
# joke.c contains the generated function
# vibec also builds joke.so for dynamic loading
gcc -o joke_app joke_app.c joke.c -lvibelang
# vibec also produces joke.so for dynamic loading
./joke_app <topic>
```
Here is a minimal `joke_app.c`:

```c
#include <runtime.h>

extern char* tellJoke(const char* topic);

int main(int argc, char **argv) {
    if (argc != 2) return 1;
    // Manual initialization is optional; the runtime will auto-start on first use
    vibe_runtime_init();
    char *joke = tellJoke(argv[1]);
    printf("%s\n", joke);
    return 0;
}
```

If `joke_app` fails to launch with a message like
`Library not loaded: @rpath/libvibelang.dylib`, make sure the runtime can
locate `libvibelang`. You can set `LD_LIBRARY_PATH` (or `DYLD_LIBRARY_PATH`
   on macOS) to the install directory or link with an rpath:

   ```bash
   gcc -o joke_app joke_app.c joke.c -lvibelang \
       -Wl,-rpath,/usr/local/lib
   ```
   The program will print a short joke about the given topic.

## Using the Python helpers

The `python` directory contains a small helper package that can compile and load
`.vibe` files directly from Python. The two main functions are
`vibelang.compile()` and `vibelang.load()`.

```python
from vibelang import compile, load

# Compile a VibeLang file using vibec
so_file = compile("joke.vibe")

# Load the shared library and call the generated function
module = load("joke.vibe")
print(module.tellJoke(b"computers"))
```

`load()` ensures the runtime library `libvibelang` is available. If it is not in
a system search path, set `VIBELANG_LIB_DIR` (or `LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH`)
to the directory containing the library.
