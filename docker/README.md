# Docker Build System

This directory contains the Docker-based build system for neufox-2FA across multiple platforms and configurations.

## Directory Structure

```
docker/
├── build.sh           # Build individual Linux variant (bash)
├── build-all.sh       # Build all Linux variants (bash)
├── linux/             # Linux build environment (Ubuntu 22.04)
│   ├── Dockerfile
│   ├── docker-entrypoint.sh
│   └── README.md
├── windows/           # Windows build environment (Server 2022)
│   ├── Dockerfile
│   ├── build-entrypoint.ps1
│   ├── build.ps1      # Build individual Windows variant
│   ├── build-all.ps1  # Build all Windows variants
│   └── README.md
└── README.md          # This file
```

## Quick Start

### Prerequisites

**Linux/macOS:**
- Docker or Podman installed
- Git (for cloning the repository)
- ~2GB disk space per Linux architecture

**Windows:**
- Docker Desktop with Windows containers (for Docker builds), OR
- Visual Studio 2022 + vcpkg (for local builds)
- Git
- ~10GB disk space (for Docker) or ~5GB (for local builds)

### Build Commands

**Linux builds:**
```bash
cd docker

# Build a specific variant
./build.sh x86_64 static

# Build all Linux variants (8 total)
./build-all.sh
```

**Windows builds:**
```powershell
cd docker\windows

# Build a specific variant (local)
.\build.ps1 x64

# Build with Docker
.\build.ps1 x64 -UseDocker

# Build all Windows variants (2 total: x64, Win32)
.\build-all.ps1
```

## Build Variants

The build system produces the following variants:

### Linux (8 variants)

| Architecture | SSL Type | Description |
|--------------|----------|-------------|
| x86          | static   | 32-bit Intel/AMD, static OpenSSL |
| x86          | dynssl   | 32-bit Intel/AMD, dynamic OpenSSL |
| x86_64       | static   | 64-bit Intel/AMD, static OpenSSL |
| x86_64       | dynssl   | 64-bit Intel/AMD, dynamic OpenSSL |
| arm32        | static   | 32-bit ARM, static OpenSSL |
| arm32        | dynssl   | 32-bit ARM, dynamic OpenSSL |
| aarch64      | static   | 64-bit ARM, static OpenSSL |
| aarch64      | dynssl   | 64-bit ARM, dynamic OpenSSL |

### Windows (2 variants)

| Architecture | Configuration | Description |
|--------------|---------------|-------------|
| x64          | RelWithDebInfo | 64-bit Windows with debug symbols |
| Win32        | RelWithDebInfo | 32-bit Windows with debug symbols |

**Note**: Windows builds use dynamic OpenSSL by default (vcpkg)

### SSL Configuration Explained

- **static**: OpenSSL is compiled into the binary
  - ✅ Pros: No runtime dependencies, works everywhere
  - ❌ Cons: Larger file size (~3-4 MB larger)
  - **Recommended for most users**

- **dynssl** (dynamic): Links to system OpenSSL
  - ✅ Pros: Smaller file size, benefits from system security updates
  - ❌ Cons: Requires OpenSSL 1.1+ on target system
  - **Recommended for modern systems**

## Output Files

Build artifacts are created in:

```
releases/
# Linux builds (.tar.gz)
├── neufox-2fa-linux-x86-static-{version}.tar.gz
├── neufox-2fa-linux-x86-static-{version}.tar.gz.sha256
├── neufox-2fa-linux-x86-dynssl-{version}.tar.gz
├── neufox-2fa-linux-x86-dynssl-{version}.tar.gz.sha256
├── neufox-2fa-linux-x86_64-static-{version}.tar.gz
├── ... (and so on for other Linux architectures)
│
# Windows builds (.zip)
├── neufox-2fa-windows-x64-{version}.zip
├── neufox-2fa-windows-x64-{version}.zip.sha256
├── neufox-2fa-windows-Win32-{version}.zip
└── neufox-2fa-windows-Win32-{version}.zip.sha256
```

- **Linux**: `.tar.gz` archives containing `.so` files
- **Windows**: `.zip` archives containing `.dll` and `.pdb` files
- Each archive includes a `.sha256` file for integrity verification

## Usage Examples

### Development Build

**Linux:**
```bash
# Quick test build for x86_64
./build.sh x86_64 static
```

**Windows:**
```powershell
# Quick test build for x64
cd windows
.\build.ps1 x64
```

### Release Build

**Linux:**
```bash
# Set version and build all Linux variants
VERSION=v1.0.0 ./build-all.sh
```

**Windows:**
```powershell
# Build all Windows variants with version
cd windows
.\build-all.ps1 -Version "v1.0.0"
```

### Custom Configuration

**Linux (Docker):**
```bash
# Debug build with dynamic SSL
docker run --rm \
    -v $(pwd)/..:/build \
    -e ARCH=x86_64 \
    -e SHARED_OPENSSL=1 \
    -e CONFIG=Debug \
    neufox-2fa-builder:x86_64
```

**Windows (PowerShell):**
```powershell
# Debug build
cd windows
.\build.ps1 Win32 Debug
```

## CI/CD Integration

These build scripts are designed for easy integration with CI/CD pipelines:

**Linux builds (GitHub Actions):**
```yaml
- name: Build all Linux variants
  run: |
    cd docker
    VERSION=${{ github.ref_name }} ./build-all.sh
    
- name: Upload Linux artifacts
  uses: actions/upload-artifact@v3
  with:
    name: linux-builds
    path: releases/*.tar.gz
```

**Windows builds (GitHub Actions):**
```yaml
- name: Build all Windows variants
  run: |
    cd docker\windows
    .\build-all.ps1 -Version "${{ github.ref_name }}"
  shell: powershell
    
- name: Upload Windows artifacts
  uses: actions/upload-artifact@v3
  with:
    name: windows-builds
    path: releases\*.zip
```

See [CI_CD.md](../CI_CD.md) for complete CI/CD setup instructions.

## Troubleshooting

### Linux: Permission Denied on Scripts

On Linux/macOS, make scripts executable:

```bash
chmod +x docker/build.sh docker/build-all.sh docker/linux/docker-entrypoint.sh
```

### Linux: Docker Build Fails

1. Check Docker is running: `docker ps`
2. Ensure you have sufficient disk space
3. Try building with `--no-cache`: `docker build --no-cache ...`

### Linux: ARM Builds on x86_64

For cross-platform ARM builds, you may need QEMU:

```bash
# On Ubuntu/Debian
sudo apt-get install qemu-user-static

# Enable binfmt
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
```

### Windows: vcpkg Not Found

```powershell
# Set VCPKG_ROOT environment variable
$env:VCPKG_ROOT = "C:\vcpkg"

# Or install vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```

### Windows: Visual Studio Not Found

Install Visual Studio 2022 with "Desktop development with C++" workload from:
https://visualstudio.microsoft.com/downloads/

### Windows: Docker Container Type Mismatch

Switch Docker Desktop to Windows containers:
- Right-click Docker tray icon → "Switch to Windows containers..."

### Build Output Not Found

- **Linux**: Ensure you're mounting the project root correctly
- **Windows**: Check vcpkg and Visual Studio are installed
- Check Docker has permission to write to the project directory
- Verify the build completed successfully (check exit code)

## Platform Support

### Tested On

- ✅ Linux (Ubuntu 20.04+, Debian 11+)
- ✅ macOS (Docker Desktop)
- ✅ Windows (WSL2 + Docker Desktop)

### Docker Versions

- Minimum: Docker 20.10+
- Recommended: Docker 24.0+

## Future Plans

- [x] ~~Add Windows build support~~ (completed)
- [ ] Add macOS build support (for open.mp macOS server if needed)
- [ ] Optimize build cache for faster rebuilds
- [ ] Add multi-stage builds to reduce Linux image size
- [ ] Support for custom compiler flags via environment variables
- [ ] Add static OpenSSL option for Windows builds

## Contributing

When adding new build configurations:

1. Update the Dockerfile and docker-entrypoint.sh
2. Update this README and the linux/README.md
3. Test the build locally before committing
4. Update CI/CD workflows if needed

## License

Same as the main project. See [../LICENSE](../LICENSE).

