#!/bin/bash

# RP2040 N64 Controller Build Script

set -e  # Exit on any error

echo "Building RP2040 N64 Controller Emulator..."

# Check if SDK path is set
if [ -z "$PICO_SDK_PATH" ]; then
    echo "Warning: PICO_SDK_PATH not set. Attempting to fetch SDK from git..."
    export PICO_SDK_FETCH_FROM_GIT=1
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring build..."
cmake ..

# Build the project
echo "Compiling..."
make -j$(nproc)

# Check if build was successful
if [ -f "n64_controller.uf2" ]; then
    echo ""
    echo "Build successful!"
    echo "Output file: build/n64_controller.uf2"
    echo ""
    echo "To flash to your Pico:"
    echo "1. Hold the BOOTSEL button while connecting USB"
    echo "2. Copy n64_controller.uf2 to the mounted drive"
    echo "3. The Pico will automatically reboot and start the program"
else
    echo "Build failed!"
    exit 1
fi 