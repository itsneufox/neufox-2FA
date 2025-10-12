# Build script for Windows Docker container
param(
    [string]$Arch = $env:ARCH,
    [string]$Config = $env:CONFIG
)

# Defaults
if (-not $Arch) { $Arch = "x64" }
if (-not $Config) { $Config = "RelWithDebInfo" }

# Validate architecture
if ($Arch -notin @("x64", "Win32", "x86")) {
    Write-Error "Unsupported architecture: $Arch. Use x64, Win32, or x86"
    exit 1
}

# Normalize architecture name
if ($Arch -eq "x86") { $Arch = "Win32" }

$vcpkgArch = if ($Arch -eq "Win32") { "x86-windows" } else { "x64-windows" }
$outputDir = "build-windows-$Arch"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Building neufox-2FA for Windows" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Architecture: $Arch" -ForegroundColor Yellow
Write-Host "Configuration: $Config" -ForegroundColor Yellow
Write-Host "vcpkg triplet: $vcpkgArch" -ForegroundColor Yellow
Write-Host "Output: $outputDir" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Install dependencies via vcpkg
Write-Host "Installing dependencies via vcpkg..." -ForegroundColor Green
$dependencies = @(
    "glm:$vcpkgArch",
    "robin-hood-hashing:$vcpkgArch",
    "span-lite:$vcpkgArch",
    "string-view-lite:$vcpkgArch",
    "openssl:$vcpkgArch"
)

foreach ($dep in $dependencies) {
    Write-Host "  - $dep" -ForegroundColor Gray
}

& vcpkg install $dependencies --clean-after-build

if ($LASTEXITCODE -ne 0) {
    Write-Error "vcpkg install failed"
    exit 1
}

Write-Host ""
Write-Host "Configuring CMake..." -ForegroundColor Green

# Configure CMake
& cmake `
    -S C:\build `
    -B "C:\build\$outputDir" `
    -G "Visual Studio 17 2022" `
    -A $Arch `
    -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake" `
    -DCMAKE_BUILD_TYPE=$Config

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed"
    exit 1
}

Write-Host ""
Write-Host "Building..." -ForegroundColor Green

# Build
& cmake `
    --build "C:\build\$outputDir" `
    --config $Config `
    --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed"
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Build complete: $outputDir" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan

# List output files
if (Test-Path "C:\build\$outputDir\$Config") {
    Write-Host ""
    Write-Host "Output files:" -ForegroundColor Green
    Get-ChildItem "C:\build\$outputDir\$Config" -File | ForEach-Object {
        Write-Host "  - $($_.Name) ($([math]::Round($_.Length / 1MB, 2)) MB)" -ForegroundColor Gray
    }
}

