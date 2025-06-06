"""Utilities for compiling VibeLang sources and loading the resulting modules."""

from __future__ import annotations

import ctypes
import os
import shutil
import subprocess
import sys
from ctypes import util
from pathlib import Path


class VibeModule:
    """Simple wrapper around a loaded VibeLang module."""

    def __init__(self, library: ctypes.CDLL, runtime: ctypes.CDLL) -> None:
        self._lib = library
        self._runtime = runtime

    def __getattr__(self, name: str):
        return getattr(self._lib, name)


def compile(source: str | os.PathLike[str], vibec: str | None = None) -> Path:
    """Compile a `.vibe` file using ``vibec`` and return the path to the ``.so``.

    If ``vibec`` fails to create the shared library, this function will attempt
    to build it directly using the system compiler.
    """
    vibec_bin = vibec or shutil.which("vibec")
    if vibec_bin is None:
        raise FileNotFoundError("vibec executable not found in PATH")

    src = Path(source)

    # Ensure the runtime library can be located by vibec
    env = os.environ.copy()
    lib_dir = env.get("VIBELANG_LIB_DIR")
    if not lib_dir:
        # Assume build layout: vibec is in <root>/build/bin
        vibec_path = Path(vibec_bin).resolve()
        lib_dir = str(vibec_path.parent.parent / "lib")
    if os.path.isdir(lib_dir):
        ld_var = "DYLD_LIBRARY_PATH" if sys.platform == "darwin" else "LD_LIBRARY_PATH"
        env[ld_var] = lib_dir + os.pathsep + env.get(ld_var, "")

    subprocess.run([vibec_bin, str(src)], check=True, env=env)

    so_path = src.with_suffix(".so")
    if not so_path.exists():
        # Fallback: manually build the shared library
        c_file = src.with_suffix(".c")
        cc = shutil.which("cc") or shutil.which("gcc")
        root = Path(__file__).resolve().parents[2]
        include_dirs = [root / "include", root / "src", root / "src" / "utils"]
        cmd = [cc, "-shared", "-fPIC", str(c_file)]
        for inc in include_dirs:
            cmd.extend(["-I", str(inc)])
        # Link against static runtime components to avoid unresolved symbols
        cmd.extend([
            str(Path(lib_dir) / "libvibelang_runtime.a"),
            str(Path(lib_dir) / "libvibelang_utils.a"),
            str(Path(lib_dir) / "libcjson.a"),
            "-lcurl",
            "-o",
            str(so_path),
        ])
        subprocess.run(cmd, check=True, env=env)
    if not so_path.exists():
        raise RuntimeError(f"Expected shared library {so_path} not produced")
    return so_path


def _load_runtime() -> ctypes.CDLL:
    """Load ``libvibelang`` and initialize the runtime."""
    libname = util.find_library("vibelang")
    runtime = None
    if libname:
        runtime = ctypes.CDLL(libname, mode=ctypes.RTLD_GLOBAL)
    else:
        lib_dir = os.environ.get("VIBELANG_LIB_DIR")
        if lib_dir:
            for ext in ("so", "dylib"):
                candidate = Path(lib_dir) / f"libvibelang.{ext}"
                if candidate.exists():
                    runtime = ctypes.CDLL(str(candidate), mode=ctypes.RTLD_GLOBAL)
                    break
    if runtime is None:
        raise OSError(
            "libvibelang not found. Install it or set VIBELANG_LIB_DIR/LD_LIBRARY_PATH"
        )
    if hasattr(runtime, "vibe_runtime_init"):
        runtime.vibe_runtime_init.restype = ctypes.c_int
        runtime.vibe_runtime_init.argtypes = []
        runtime.vibe_runtime_init()
    return runtime


def load(path: str | os.PathLike[str], vibec: str | None = None) -> VibeModule:
    """Compile (if needed) and load a VibeLang module.

    ``path`` can point to either a ``.vibe`` source file or an already compiled
    ``.so`` file. The returned object exposes the C functions via ``ctypes``.
    """
    file_path = Path(path)
    if file_path.suffix == ".vibe" or not file_path.exists():
        file_path = compile(file_path.with_suffix(".vibe"), vibec=vibec)

    runtime = _load_runtime()
    lib = ctypes.CDLL(str(file_path))
    return VibeModule(lib, runtime)
