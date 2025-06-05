# VibeLang VS Code Extension

This extension provides basic syntax highlighting for `.vibe` files.

## Usage

1. Open the `vscode` folder in VS Code.
2. Press `F5` to launch a new Extension Development Host.
3. Open a VibeLang file (`*.vibe`) to see highlighted syntax.

## Building the VSIX Package

To generate a `.vsix` file for local installation:

```bash
npm install -g @vscode/vsce  # if you don't have vsce yet
vsce package
```

This creates `vibelang-0.0.1.vsix` in this directory. You can install it by
running `code --install-extension vibelang-0.0.1.vsix` or via **Extensions >
Install from VSIX...** inside VS Code.

This extension is minimal and only includes highlighting rules. Additional
features like code completion and diagnostics may be added in the future.