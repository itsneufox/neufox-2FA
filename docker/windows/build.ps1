# Windows build script for neufox-2FA
# Can be run locally or with Docker
param(
    [Parameter(Position=0)]
    [ValidateSet("x64", "Win32", "x86")]
    [string]$Arch = "x64",
    
    [Parameter(Position=1)]
    [ValidateSet("Release", "RelWithDebInfo", "Debug")]
    [string]$Config = "RelWithDebInfo",
    
    [switch]$UseDocker,
    [string]$Version = "dev"
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent (Split-Path -Parent $scriptDir)

# Normalize architecture
if ($Arch -eq "x86") { $Arch = "Win32" }

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Building neufox-2FA for Windows" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Architecture: $Arch" -ForegroundColor Yellow
Write-Host "Configuration: $Config" -ForegroundColor Yellow
Write-Host "Version: $Version" -ForegroundColor Yellow
Write-Host "Use Docker: $UseDocker" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if ($UseDocker) {
    # Docker build
    Write-Host "Building with Docker..." -ForegroundColor Green
    
    $imageName = "neufox-2fa-builder-windows"
    
    # Build Docker image
    Write-Host "Building Docker image..." -ForegroundColor Green
    docker build `
        --build-arg ARCH=$Arch `
        -t "${imageName}:${Arch}" `
        -f "$scriptDir\Dockerfile" `
        $scriptDir
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Docker image build failed"
        exit 1
    }
    
    # Run build in container
    Write-Host ""
    Write-Host "Running build in container..." -ForegroundColor Green
    docker run --rm `
        -v "${projectRoot}:C:\build" `
        -e ARCH=$Arch `
        -e CONFIG=$Config `
        "${imageName}:${Arch}"
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Docker build failed"
        exit 1
    }
} else {
    # Local build (requires vcpkg and Visual Studio)
    Write-Host "Building locally..." -ForegroundColor Green
    
    # Check for vcpkg
    $vcpkgRoot = $env:VCPKG_ROOT
    if (-not $vcpkgRoot) {
        Write-Error "VCPKG_ROOT environment variable not set. Please install vcpkg or use -UseDocker"
        Write-Host "Install vcpkg: git clone https://github.com/Microsoft/vcpkg.git && cd vcpkg && .\bootstrap-vcpkg.bat"
        exit 1
    }
    
    if (-not (Test-Path $vcpkgRoot)) {
        Write-Error "vcpkg not found at: $vcpkgRoot"
        exit 1
    }
    
    $vcpkgArch = if ($Arch -eq "Win32") { "x86-windows" } else { "x64-windows" }
    $outputDir = "build-windows-$Arch"
    
    # Install dependencies
    Write-Host "Installing dependencies via vcpkg (static linking)..." -ForegroundColor Green
    $vcpkgArchStatic = "$vcpkgArch-static"
    $dependencies = @(
        "glm:$vcpkgArchStatic",
        "robin-hood-hashing:$vcpkgArchStatic",
        "span-lite:$vcpkgArchStatic",
        "string-view-lite:$vcpkgArchStatic",
        "openssl:$vcpkgArchStatic"
    )
    
    foreach ($dep in $dependencies) {
        Write-Host "  - $dep" -ForegroundColor Gray
    }
    
    & "$vcpkgRoot\vcpkg.exe" install @dependencies --clean-after-build
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "vcpkg install failed"
        exit 1
    }
    
    # Configure CMake
    Write-Host ""
    Write-Host "Configuring CMake..." -ForegroundColor Green
    
    & cmake `
        -S $projectRoot `
        -B "$projectRoot\$outputDir" `
        -G "Visual Studio 17 2022" `
        -A $Arch `
        -DCMAKE_TOOLCHAIN_FILE="$vcpkgRoot\scripts\buildsystems\vcpkg.cmake" `
        -DCMAKE_BUILD_TYPE=$Config
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake configuration failed"
        exit 1
    }
    
    # Build
    Write-Host ""
    Write-Host "Building..." -ForegroundColor Green
    
    & cmake `
        --build "$projectRoot\$outputDir" `
        --config $Config `
        --parallel
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed"
        exit 1
    }
}

# Package the build
$outputDir = "build-windows-$Arch"
$outputName = "neufox-2fa-windows-${Arch}-${Version}"
$releasesDir = "$projectRoot\releases"

if (Test-Path "$projectRoot\$outputDir\$Config") {
    Write-Host ""
    Write-Host "Packaging build..." -ForegroundColor Green
    
    # Create releases directory
    if (-not (Test-Path $releasesDir)) {
        New-Item -ItemType Directory -Path $releasesDir | Out-Null
    }
    
    # Create zip archive
    $zipPath = "$releasesDir\${outputName}.zip"
    Compress-Archive `
        -Path "$projectRoot\$outputDir\$Config\*" `
        -DestinationPath $zipPath `
        -Force
    
    # Calculate SHA256
    $hash = Get-FileHash -Path $zipPath -Algorithm SHA256
    $hash.Hash.ToLower() | Out-File -FilePath "$zipPath.sha256" -Encoding ASCII
    
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Build completed successfully!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Output: releases\${outputName}.zip" -ForegroundColor Yellow
    Write-Host "SHA256: releases\${outputName}.zip.sha256" -ForegroundColor Yellow
    
    # Show file size
    $zipFile = Get-Item $zipPath
    Write-Host "Size: $([math]::Round($zipFile.Length / 1MB, 2)) MB" -ForegroundColor Yellow
    
    # List contents
    Write-Host ""
    Write-Host "Package contents:" -ForegroundColor Green
    Get-ChildItem "$projectRoot\$outputDir\$Config" -File | ForEach-Object {
        Write-Host "  - $($_.Name)" -ForegroundColor Gray
    }
} else {
    Write-Error "Build output directory not found: $projectRoot\$outputDir\$Config"
    exit 1
}

