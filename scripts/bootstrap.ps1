# ==============================================================================
# Severance Development Environment Bootstrap (Windows)
# ==============================================================================
# This script installs all required tools for building Severance.
# Run from an elevated (Administrator) PowerShell terminal.
#
# What it installs:
#   1. Visual Studio 2022 Build Tools (C++23 compiler)
#   2. CMake 3.28+
#   3. vcpkg (C++ package manager)
#   4. Qt 6.5+ (via vcpkg)
#
# Usage:
#   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
#   .\scripts\bootstrap.ps1
# ==============================================================================

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host "  $Message" -ForegroundColor White
    Write-Host "========================================================" -ForegroundColor Cyan
}

function Write-Ok {
    param([string]$Message)
    Write-Host "  [OK] $Message" -ForegroundColor Green
}

function Write-Skip {
    param([string]$Message)
    Write-Host "  [SKIP] $Message (already installed)" -ForegroundColor Yellow
}

function Write-Info {
    param([string]$Message)
    Write-Host "  $Message" -ForegroundColor Gray
}

function Write-Fail {
    param([string]$Message)
    Write-Host "  [FAIL] $Message" -ForegroundColor Red
}

# -------------------------------------------------------
# Check if running as Administrator
# -------------------------------------------------------
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host ""
    Write-Host "  WARNING: This script should be run as Administrator." -ForegroundColor Yellow
    Write-Host "  Some steps may fail without elevation." -ForegroundColor Yellow
    Write-Host ""
    $continue = Read-Host "  Continue anyway? (y/N)"
    if ($continue -ne 'y') { exit 0 }
}

# -------------------------------------------------------
# Step 1: Visual Studio 2022 Build Tools
# -------------------------------------------------------
Write-Step "Step 1: Visual Studio 2022 Build Tools"

$vs2022Found = $false
$vs2022Paths = @(
    "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools",
    "C:\Program Files\Microsoft Visual Studio\2022\Community",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise",
    "C:\Program Files\Microsoft Visual Studio\2022\BuildTools"
)
foreach ($p in $vs2022Paths) {
    if (Test-Path $p) { $vs2022Found = $true; break }
}

if ($vs2022Found) {
    Write-Skip "Visual Studio 2022 detected"
} else {
    Write-Info "Installing Visual Studio 2022 Build Tools with C++ workload..."
    Write-Info "This may take 10-20 minutes."
    $overrideArgs = "--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows11SDK.22621 --includeRecommended"
    winget install --id Microsoft.VisualStudio.2022.BuildTools --accept-source-agreements --accept-package-agreements --override $overrideArgs

    if ($LASTEXITCODE -eq 0) {
        Write-Ok "Visual Studio 2022 Build Tools installed"
    } else {
        Write-Fail "Installation returned non-zero exit code."
        Write-Info "You can install manually from: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022"
    }
}

# -------------------------------------------------------
# Step 2: CMake
# -------------------------------------------------------
Write-Step "Step 2: CMake"

$cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmakeCmd) {
    $cmakeVer = & cmake --version 2>&1 | Select-Object -First 1
    Write-Skip "CMake found: $cmakeVer"
} else {
    Write-Info "Installing CMake via winget..."
    winget install --id Kitware.CMake --accept-source-agreements --accept-package-agreements

    if ($LASTEXITCODE -eq 0) {
        Write-Ok "CMake installed"
        Write-Info "NOTE: You may need to restart your terminal for cmake to appear in PATH"
    } else {
        Write-Fail "CMake installation failed"
    }
}

# -------------------------------------------------------
# Step 3: vcpkg
# -------------------------------------------------------
Write-Step "Step 3: vcpkg (C++ Package Manager)"

$vcpkgDir = "C:\vcpkg"
if (Test-Path "$vcpkgDir\vcpkg.exe") {
    Write-Skip "vcpkg found at $vcpkgDir"
} else {
    Write-Info "Cloning vcpkg to $vcpkgDir..."
    git clone https://github.com/microsoft/vcpkg.git $vcpkgDir

    Write-Info "Bootstrapping vcpkg..."
    & "$vcpkgDir\bootstrap-vcpkg.bat" -disableMetrics

    if (Test-Path "$vcpkgDir\vcpkg.exe") {
        Write-Ok "vcpkg installed at $vcpkgDir"
    } else {
        Write-Fail "vcpkg bootstrap failed"
    }
}

# Set VCPKG_ROOT environment variable
$currentVcpkgRoot = [System.Environment]::GetEnvironmentVariable("VCPKG_ROOT", "User")
if ($currentVcpkgRoot -ne $vcpkgDir) {
    Write-Info "Setting VCPKG_ROOT environment variable..."
    [System.Environment]::SetEnvironmentVariable("VCPKG_ROOT", $vcpkgDir, "User")
    $env:VCPKG_ROOT = $vcpkgDir
    Write-Ok "VCPKG_ROOT set to $vcpkgDir"
}

# Add vcpkg to PATH if not already there
$userPath = [System.Environment]::GetEnvironmentVariable("Path", "User")
if ($userPath -and ($userPath -notlike "*$vcpkgDir*")) {
    Write-Info "Adding vcpkg to user PATH..."
    [System.Environment]::SetEnvironmentVariable("Path", "$userPath;$vcpkgDir", "User")
    $env:Path = "$env:Path;$vcpkgDir"
    Write-Ok "vcpkg added to PATH"
}

# -------------------------------------------------------
# Step 4: Install vcpkg dependencies
# -------------------------------------------------------
Write-Step "Step 4: Installing C++ Dependencies via vcpkg"

$vcpkgExe = "$vcpkgDir\vcpkg.exe"

if (Test-Path $vcpkgExe) {
    Write-Info "Installing spdlog, catch2, and qtbase..."
    Write-Info "Qt 6 builds from source via vcpkg -- expect 15-30 minutes on first install."

    & $vcpkgExe install spdlog:x64-windows catch2:x64-windows qtbase:x64-windows

    if ($LASTEXITCODE -eq 0) {
        Write-Ok "All vcpkg packages installed"
    } else {
        Write-Fail "Some packages failed to install. Run manually:"
        Write-Info "  $vcpkgExe install spdlog:x64-windows catch2:x64-windows qtbase:x64-windows"
    }
} else {
    Write-Fail "vcpkg not found -- skipping dependency install"
}

# -------------------------------------------------------
# Summary
# -------------------------------------------------------
Write-Step "Setup Summary"

$vcpkgDir = "C:\vcpkg"
$allGood = $true

# Check Git
if (Get-Command git -ErrorAction SilentlyContinue) {
    Write-Ok "Git"
} else {
    Write-Fail "Git -- NOT FOUND"
    $allGood = $false
}

# Check VS 2022
$vs2022Check = $false
foreach ($p in $vs2022Paths) {
    if (Test-Path $p) { $vs2022Check = $true; break }
}
if ($vs2022Check) {
    Write-Ok "VS 2022 Build Tools"
} else {
    Write-Fail "VS 2022 Build Tools -- NOT FOUND"
    $allGood = $false
}

# Check CMake
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    Write-Ok "CMake"
} else {
    Write-Fail "CMake -- NOT FOUND (may need terminal restart)"
    $allGood = $false
}

# Check vcpkg
if (Test-Path "$vcpkgDir\vcpkg.exe") {
    Write-Ok "vcpkg"
} else {
    Write-Fail "vcpkg -- NOT FOUND"
    $allGood = $false
}

Write-Host ""
if ($allGood) {
    Write-Host "  All tools installed! You are ready to build Severance." -ForegroundColor Green
    Write-Host ""
    Write-Host "  Next steps:" -ForegroundColor White
    Write-Host "    1. Open a NEW terminal (to pick up PATH changes)" -ForegroundColor Gray
    Write-Host "    2. cd to the severance project directory" -ForegroundColor Gray
    Write-Host "    3. cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake" -ForegroundColor Gray
    Write-Host "    4. cmake --build build --config Debug" -ForegroundColor Gray
    Write-Host "    5. .\build\Debug\severance.exe" -ForegroundColor Gray
} else {
    Write-Host "  Some tools are missing. Please install them manually and re-run." -ForegroundColor Yellow
}
Write-Host ""
