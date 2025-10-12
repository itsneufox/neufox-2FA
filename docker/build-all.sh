#!/bin/bash
set -e

# Build script for all neufox-2FA Linux variants
# Builds for multiple architectures with dynamic and static SSL

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

IMAGE_NAME="neufox-2fa-builder"
VERSION="${VERSION:-dev}"

# Architectures to build
ARCHITECTURES=("x86" "x86_64" "arm32" "aarch64")

# SSL configurations: 0=static, 1=dynamic
SSL_CONFIGS=("0" "1")

echo "========================================"
echo "Building neufox-2FA for all platforms"
echo "========================================"
echo "Version: $VERSION"
echo "Architectures: ${ARCHITECTURES[*]}"
echo "SSL Configs: static, dynamic"
echo "========================================"
echo ""

# Build Docker image for each architecture
for arch in "${ARCHITECTURES[@]}"; do
    echo "Building Docker image for $arch..."
    docker build \
        --build-arg ARCH="$arch" \
        -t "$IMAGE_NAME:$arch" \
        -f "$SCRIPT_DIR/linux/Dockerfile" \
        "$SCRIPT_DIR/linux"
    echo ""
done

# Create output directory
mkdir -p "$PROJECT_ROOT/releases"

# Build all variants
for arch in "${ARCHITECTURES[@]}"; do
    for ssl in "${SSL_CONFIGS[@]}"; do
        if [ "$ssl" = "1" ]; then
            ssl_type="dynssl"
        else
            ssl_type="static"
        fi
        
        output_name="neufox-2fa-linux-${arch}-${ssl_type}-${VERSION}"
        
        echo "========================================"
        echo "Building: $output_name"
        echo "========================================"
        
        docker run --rm \
            -v "$PROJECT_ROOT:/build" \
            -e ARCH="$arch" \
            -e SHARED_OPENSSL="$ssl" \
            -e CONFIG="Release" \
            "$IMAGE_NAME:$arch"
        
        # Package the build
        build_dir="$PROJECT_ROOT/build-${arch}-${ssl_type}"
        if [ -d "$build_dir" ]; then
            echo "Packaging $output_name..."
            tar -czf "$PROJECT_ROOT/releases/${output_name}.tar.gz" \
                -C "$build_dir" \
                .
            
            # Calculate SHA256
            sha256sum "$PROJECT_ROOT/releases/${output_name}.tar.gz" \
                > "$PROJECT_ROOT/releases/${output_name}.tar.gz.sha256"
            
            echo "Created: releases/${output_name}.tar.gz"
            echo ""
        fi
    done
done

echo "========================================"
echo "All builds completed!"
echo "========================================"
echo "Release files in: releases/"
ls -lh "$PROJECT_ROOT/releases/"

