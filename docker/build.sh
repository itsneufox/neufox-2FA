#!/bin/bash
set -e

# Build script for individual neufox-2FA Linux variant
# Usage: ./build.sh [architecture] [ssl-type]
# Example: ./build.sh x86 dynssl

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Parse arguments
ARCH="${1:-x86}"
SSL_TYPE="${2:-static}"

# Validate architecture
case "$ARCH" in
    x86|x86_64|arm32|aarch64)
        ;;
    *)
        echo "Error: Unsupported architecture '$ARCH'"
        echo "Supported architectures: x86, x86_64, arm32, aarch64"
        exit 1
        ;;
esac

# Validate SSL type
case "$SSL_TYPE" in
    static)
        SHARED_OPENSSL=0
        ;;
    dynssl|dynamic)
        SHARED_OPENSSL=1
        SSL_TYPE="dynssl"
        ;;
    *)
        echo "Error: Unsupported SSL type '$SSL_TYPE'"
        echo "Supported types: static, dynssl (dynamic)"
        exit 1
        ;;
esac

IMAGE_NAME="neufox-2fa-builder"
VERSION="${VERSION:-dev}"
OUTPUT_NAME="neufox-2fa-linux-${ARCH}-${SSL_TYPE}-${VERSION}"

echo "========================================"
echo "Building neufox-2FA"
echo "========================================"
echo "Architecture: $ARCH"
echo "SSL Type: $SSL_TYPE"
echo "Version: $VERSION"
echo "========================================"
echo ""

# Build Docker image
echo "Building Docker image..."
docker build \
    --build-arg ARCH="$ARCH" \
    -t "$IMAGE_NAME:$ARCH" \
    -f "$SCRIPT_DIR/linux/Dockerfile" \
    "$SCRIPT_DIR/linux"

echo ""
echo "Running build..."

# Run build
docker run --rm \
    -v "$PROJECT_ROOT:/build" \
    -e ARCH="$ARCH" \
    -e SHARED_OPENSSL="$SHARED_OPENSSL" \
    -e CONFIG="Release" \
    "$IMAGE_NAME:$ARCH"

# Package the build
BUILD_DIR="$PROJECT_ROOT/build-${ARCH}-${SSL_TYPE}"
if [ -d "$BUILD_DIR" ]; then
    mkdir -p "$PROJECT_ROOT/releases"
    
    echo ""
    echo "Packaging $OUTPUT_NAME..."
    tar -czf "$PROJECT_ROOT/releases/${OUTPUT_NAME}.tar.gz" \
        -C "$BUILD_DIR" \
        .
    
    # Calculate SHA256
    sha256sum "$PROJECT_ROOT/releases/${OUTPUT_NAME}.tar.gz" \
        > "$PROJECT_ROOT/releases/${OUTPUT_NAME}.tar.gz.sha256"
    
    echo "========================================"
    echo "Build completed successfully!"
    echo "========================================"
    echo "Output: releases/${OUTPUT_NAME}.tar.gz"
    echo "SHA256: releases/${OUTPUT_NAME}.tar.gz.sha256"
    
    # Show file size
    ls -lh "$PROJECT_ROOT/releases/${OUTPUT_NAME}.tar.gz"
else
    echo "Error: Build directory not found: $BUILD_DIR"
    exit 1
fi

