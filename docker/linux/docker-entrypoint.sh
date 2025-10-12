#!/bin/bash
set -e

# Configuration
CONFIG=${CONFIG:-Release}
ARCH=${ARCH:-x86}
SHARED_OPENSSL=${SHARED_OPENSSL:-0}

# Architecture-specific flags
case "$ARCH" in
    x86)
        CMAKE_C_FLAGS="-m32"
        CMAKE_CXX_FLAGS="-m32"
        ;;
    x86_64)
        CMAKE_C_FLAGS="-m64"
        CMAKE_CXX_FLAGS="-m64"
        ;;
    arm32)
        CMAKE_C_FLAGS="--target=arm-linux-gnueabihf"
        CMAKE_CXX_FLAGS="--target=arm-linux-gnueabihf"
        CMAKE_TOOLCHAIN_FILE="/build/cmake/arm32-toolchain.cmake"
        ;;
    aarch64)
        CMAKE_C_FLAGS="--target=aarch64-linux-gnu"
        CMAKE_CXX_FLAGS="--target=aarch64-linux-gnu"
        CMAKE_TOOLCHAIN_FILE="/build/cmake/aarch64-toolchain.cmake"
        ;;
    *)
        echo "Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

# SSL configuration
if [ "$SHARED_OPENSSL" = "1" ]; then
    STATIC_STDCXX="false"
    SSL_TYPE="dynssl"
else
    STATIC_STDCXX="true"
    SSL_TYPE="static"
fi

# Output directory
OUTPUT_DIR="build-${ARCH}-${SSL_TYPE}"

echo "=================================="
echo "Building neufox-2FA"
echo "=================================="
echo "Architecture: $ARCH"
echo "Configuration: $CONFIG"
echo "OpenSSL: $([ "$SHARED_OPENSSL" = "1" ] && echo "Dynamic" || echo "Static")"
echo "Output: $OUTPUT_DIR"
echo "=================================="

# Configure CMake
cmake \
    -S /build \
    -B "/build/$OUTPUT_DIR" \
    -G Ninja \
    -DCMAKE_C_FLAGS="$CMAKE_C_FLAGS" \
    -DCMAKE_CXX_FLAGS="$CMAKE_CXX_FLAGS" \
    -DCMAKE_BUILD_TYPE="$CONFIG" \
    -DSHARED_OPENSSL="$SHARED_OPENSSL" \
    -DSTATIC_STDCXX="$STATIC_STDCXX" \
    ${CMAKE_TOOLCHAIN_FILE:+-DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE"}

# Build
cmake \
    --build "/build/$OUTPUT_DIR" \
    --config "$CONFIG" \
    --parallel $(nproc)

echo "=================================="
echo "Build complete: $OUTPUT_DIR"
echo "=================================="
