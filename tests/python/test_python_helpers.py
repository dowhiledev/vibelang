import os
import sys
from pathlib import Path

# Add the project's Python package to the import path
root = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(root / "python"))

from vibelang import compile, load


def main():
    vibec = root / "build" / "bin" / "vibec"
    lib_dir = root / "build" / "lib"
    os.environ["VIBELANG_LIB_DIR"] = str(lib_dir)

    so_path = compile(root / "examples" / "joke.vibe", vibec=str(vibec))
    assert so_path.exists(), "Shared library not produced"

    module = load(so_path, vibec=str(vibec))
    assert hasattr(module, "tellJoke"), "Expected symbol not found"
    print("Python helpers test passed")

    # Clean up generated files
    c_file = so_path.with_suffix(".c")
    if c_file.exists():
        c_file.unlink()
    if so_path.exists():
        so_path.unlink()


if __name__ == "__main__":
    main()
