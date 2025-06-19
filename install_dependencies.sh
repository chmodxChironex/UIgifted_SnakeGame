#!/bin/bash
#
# @file install_dependencies.sh
# @brief Installs Raylib and other dependencies for the Snake game on Debian/Ubuntu.
# @author chmodxChironex
# @date 2025

set -e

echo "--- Snake Game Dependency Installer ---"

# Ensure essential build tools are present
echo "[1/3] Installing essential build tools (build-essential, cmake, pkg-config)..."
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config dos2unix

# Attempt to install Raylib from the official repository first
echo "[2/3] Attempting to install Raylib library from APT..."
if sudo apt-get install -y libraylib-dev &>/dev/null; then
    echo "      > Success: Raylib (libraylib-dev) installed from APT."
else
    echo "      > Info: libraylib-dev not found in APT. Attempting to build from source."
    echo "      > This may take a few minutes..."
    
    # Fallback: Install Raylib from source if the package is not available
    cd /tmp
    rm -rf raylib
    
    # Use a stable version known to work well
    echo "      > Downloading Raylib v4.5.0..."
    wget -q https://github.com/raysan5/raylib/archive/refs/tags/4.5.0.tar.gz
    tar -xzf 4.5.0.tar.gz
    cd raylib-4.5.0
    
    echo "      > Building Raylib from source with CMake..."
    mkdir -p build && cd build
    cmake .. -DBUILD_SHARED_LIBS=ON -DBUILD_EXAMPLES=OFF &>/dev/null
    make -j$(nproc) &>/dev/null
    
    echo "      > Installing Raylib to /usr/local/lib..."
    sudo make install &>/dev/null
    sudo ldconfig
    echo "      > Success: Raylib v4.5.0 built and installed from source."
fi

# Final check
echo "[3/3] Verifying Raylib installation..."
if pkg-config --exists raylib; then
    VERSION=$(pkg-config --modversion raylib)
    echo "      > Verification successful! Raylib version $VERSION is ready."
else
    echo "      > Error: Raylib installation could not be verified by pkg-config." >&2
    echo "      > Please check the installation logs for errors." >&2
    exit 1
fi

echo "--- Installation complete! You can now build the game using './build.sh' or 'make'. ---"