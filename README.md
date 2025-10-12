# neufox's 2FA - Two-Factor Authentication for open.mp

A secure TOTP (Time-based One-Time Password) two-factor authentication component for open.mp servers.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MPL--2.0-green)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey)
[![Build](https://github.com/itsneufox/neufox-2fa/actions/workflows/build.yml/badge.svg)](https://github.com/itsneufox/neufox-2fa/actions/workflows/build.yml)

## Downloads

Pre-built releases are available on the [Releases](../../releases) page.

## Required Tools

* [CMake 3.19+](https://cmake.org/)
* [Visual Studio 2022](https://www.visualstudio.com/) (on Windows)
* [vcpkg](https://vcpkg.io/) (on Windows, for local builds)
* Clang (on Linux)
* [Ninja](https://ninja-build.org/) (on Linux)

Visual Studio needs the `Desktop development with C++` workload.

## Sources

```bash
# Clone with submodules:
git clone --recursive https://github.com/itsneufox/neufox-2FA
cd neufox-2FA
```

Note: The `--recursive` flag is required to fetch the open.mp SDK and dependencies.

## Building on Windows

### Local Build

Prerequisites:
- Install [vcpkg](https://vcpkg.io/en/getting-started)
- Set `VCPKG_ROOT` environment variable

```powershell
cd docker\windows
.\build.ps1 x64
```

For 32-bit:
```powershell
.\build.ps1 Win32
```

### Docker Build

```powershell
cd docker\windows
.\build.ps1 x64 -UseDocker
```

Output: `releases\neufox-2fa-windows-x64-{version}.zip`

## Building on Linux

### Docker Build

Build specific variant:
```bash
cd docker
./build.sh x86_64 static
```

Build all variants:
```bash
./build-all.sh
```

### Manual Build

Prerequisites:
```bash
sudo apt-get update
sudo apt-get install -y git cmake build-essential libssl-dev ninja-build
```

For 64-bit static:
```bash
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_C_FLAGS=-m64 -DCMAKE_CXX_FLAGS=-m64 \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DSHARED_OPENSSL=OFF -DSTATIC_STDCXX=ON
cmake --build . --parallel
```

For 32-bit:
```bash
# Install 32-bit libraries
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install -y gcc-multilib g++-multilib libssl-dev:i386

# Build
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 \
         -DCMAKE_BUILD_TYPE=RelWithDebInfo \
         -DSHARED_OPENSSL=OFF -DSTATIC_STDCXX=ON
cmake --build . --parallel
```

## Build Variants

### Linux
- `neufox-2fa-x86-static.so` - 32-bit Intel/AMD, static OpenSSL
- `neufox-2fa-x86-dynssl.so` - 32-bit Intel/AMD, dynamic OpenSSL
- `neufox-2fa-x64-static.so` - 64-bit Intel/AMD, static OpenSSL
- `neufox-2fa-x64-dynssl.so` - 64-bit Intel/AMD, dynamic OpenSSL

### Windows
- `neufox-2fa-x86.dll` - 32-bit (static OpenSSL)
- `neufox-2fa-x64.dll` - 64-bit (static OpenSSL)

## Release Process

Releases are created manually using GitHub Actions with version management:

1. Go to [Actions → Release](../../actions/workflows/release.yml)
2. Click "Run workflow"
3. Enter the base version (e.g., `1.0.0`, `1.1.0`)
4. Select release type:
   - `beta` - Beta pre-release (for testing)
   - `release-candidate` - Release Candidate (for final testing)
   - `stable` - Stable release
5. Click "Run workflow"

The build number (total commit count) is automatically appended to create the final version (e.g., `v1.0.123`).

**Example:**
- Base version: `1.0.0`
- Build number: `123`
- Final version: `v1.0.123`
- Release name: `Beta v1.0.123` or `RC v1.0.123` or `v1.0.123` (for stable)

All builds are packaged with the proper folder structure:
```
neufox-2fa-{platform}-{arch}-{version}.zip
├── components/
│   └── neufox-2fa-*.dll/.so
└── include/
    └── neufox-2fa.inc
```

## Installation

1. Download the appropriate build for your platform
2. Place the component in your server's `components/` directory
3. Copy `include/neufox-2fa.inc` to your `pawno/include/` directory
4. Include in your gamemode: `#include <neufox-2fa>`

## Usage

See the [wiki](../../wiki) for detailed documentation and examples.

## Example Gamemode

For a complete working implementation, check out the [omp-base-script-with-2FA](https://github.com/itsneufox/omp-base-script-with-2FA) - a practical example showing how to integrate this component with MySQL, handle callbacks, and implement the full 2FA flow.

## License

This project is licensed under the Mozilla Public License 2.0. See [LICENSE](LICENSE) for details.

## Contributing

Contributions are welcome! Please open an issue or pull request.

## Support

For issues, questions, or feature requests, please use the [GitHub Issues](../../issues) page.
