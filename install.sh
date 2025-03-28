#!/bin/bash

# Installation script for VibeLanguage

# Make script exit if any command fails
set -e

# Create build directory if it doesn't exist
echo "Creating build directory..."
mkdir -p build
cd build

# Configure and build
echo "Configuring build..."
cmake ..

echo "Building VibeLanguage..."
make

# Install to /usr/local by default or to specified location
INSTALL_DIR=${1:-/usr/local}
echo "Installing to $INSTALL_DIR..."

# Create directories if they don't exist
sudo mkdir -p $INSTALL_DIR/bin
sudo mkdir -p $INSTALL_DIR/lib
sudo mkdir -p $INSTALL_DIR/include

# Copy the executable
echo "Installing vibec executable..."
sudo cp bin/vibec $INSTALL_DIR/bin/

# Copy the library
echo "Installing libvibelang.dylib..."
sudo cp lib/libvibelang.dylib $INSTALL_DIR/lib/

# Copy include files
echo "Installing header files..."
sudo cp ../include/vibelang.h $INSTALL_DIR/include/
sudo cp ../include/runtime.h $INSTALL_DIR/include/

# On macOS, we need to update the install name
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Updating dynamic library references..."
    sudo install_name_tool -change @rpath/libvibelang.dylib $INSTALL_DIR/lib/libvibelang.dylib $INSTALL_DIR/bin/vibec
fi

echo "Installation completed!"
echo "You can now run 'vibec' from the command line."
