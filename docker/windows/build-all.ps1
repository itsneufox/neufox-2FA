# Build all Windows variants of neufox-2FA
param(
    [ValidateSet("Release", "RelWithDebInfo", "Debug")]
    [string]$Config = "RelWithDebInfo",
    
    [switch]$UseDocker,
    [string]$Version = "dev"
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

$architectures = @("x64", "Win32")

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Building all Windows variants" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Architectures: $($architectures -join ', ')" -ForegroundColor Yellow
Write-Host "Configuration: $Config" -ForegroundColor Yellow
Write-Host "Version: $Version" -ForegroundColor Yellow
Write-Host "Use Docker: $UseDocker" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$startTime = Get-Date
$successfulBuilds = @()
$failedBuilds = @()

foreach ($arch in $architectures) {
    Write-Host ""
    Write-Host "========================================"  -ForegroundColor Magenta
    Write-Host "Building for $arch..." -ForegroundColor Magenta
    Write-Host "========================================"  -ForegroundColor Magenta
    
    try {
        $buildScript = Join-Path $scriptDir "build.ps1"
        $params = @{
            Arch = $arch
            Config = $Config
            Version = $Version
        }
        
        if ($UseDocker) {
            $params.UseDocker = $true
        }
        
        & $buildScript @params
        
        if ($LASTEXITCODE -eq 0) {
            $successfulBuilds += $arch
            Write-Host "✓ $arch build completed" -ForegroundColor Green
        } else {
            $failedBuilds += $arch
            Write-Warning "✗ $arch build failed"
        }
    } catch {
        $failedBuilds += $arch
        Write-Warning "✗ $arch build failed: $_"
    }
    
    Write-Host ""
}

$endTime = Get-Date
$duration = $endTime - $startTime

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Build Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Total time: $($duration.ToString('mm\:ss'))" -ForegroundColor Yellow
Write-Host ""

if ($successfulBuilds.Count -gt 0) {
    Write-Host "Successful builds ($($successfulBuilds.Count)):" -ForegroundColor Green
    foreach ($arch in $successfulBuilds) {
        Write-Host "  ✓ $arch" -ForegroundColor Green
    }
}

if ($failedBuilds.Count -gt 0) {
    Write-Host ""
    Write-Host "Failed builds ($($failedBuilds.Count)):" -ForegroundColor Red
    foreach ($arch in $failedBuilds) {
        Write-Host "  ✗ $arch" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Release files:" -ForegroundColor Yellow
$projectRoot = Split-Path -Parent (Split-Path -Parent $scriptDir)
$releasesDir = "$projectRoot\releases"

if (Test-Path $releasesDir) {
    Get-ChildItem $releasesDir -Filter "neufox-2fa-windows-*-${Version}.*" | ForEach-Object {
        $sizeMB = [math]::Round($_.Length / 1MB, 2)
        Write-Host "  - $($_.Name) ($sizeMB MB)" -ForegroundColor Gray
    }
} else {
    Write-Host "  No releases directory found" -ForegroundColor Gray
}

Write-Host "========================================" -ForegroundColor Cyan

# Exit with error if any builds failed
if ($failedBuilds.Count -gt 0) {
    exit 1
}

