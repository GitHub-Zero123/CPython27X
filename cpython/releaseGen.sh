#!/bin/sh
set -e  # Exit immediately if a command exits with a non-zero status.

# Clean up previous builds
if [ -f Makefile ]; then
    echo "Cleaning previous build..."
    make distclean
fi

# Configuration for Release build
# --enable-optimizations: Enable expensive optimizations (PGO)
# --with-lto: Enable Link Time Optimization
# --enable-shared: Build shared Python library
# --with-ensurepip: Install pip
echo "Configuring Release build..."
./configure \
    --enable-optimizations \
    --with-lto \
    --with-pymalloc

# Build phase
# Get number of cores for parallel build
NPROC=$(nproc 2>/dev/null || echo 2)
echo "Building with $NPROC cores..."
make -j"$NPROC"

# Install phase
# echo "Installing to $(pwd)/install_release..."
# make install

# echo "Build complete! Python is installed in $(pwd)/install_release"
