# VibeLang Python SDK

This package contains helper utilities for compiling and loading VibeLang modules directly from Python. It is managed with [Poetry](https://python-poetry.org/).

## Installation

From this directory run:

```bash
poetry install
```

This installs the package in a virtual environment created under `.venv`. You may also install it with `pip` directly:

```bash
pip install -e .
```

## Usage

```python
from vibelang import compile, load

# Build a VibeLang source file using vibec
so_file = compile("joke.vibe")

# Load the module and call a generated function
module = load(so_file)
print(module.tellJoke(b"computers"))
```

Ensure the runtime library `libvibelang` can be located by setting `VIBELANG_LIB_DIR` or adjusting `LD_LIBRARY_PATH`/`DYLD_LIBRARY_PATH` accordingly.
