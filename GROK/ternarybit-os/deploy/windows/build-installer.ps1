<#
.SYNOPSIS
    Builds the TernaryBit OS Windows installer package.
.DESCRIPTION
    This script builds the TernaryBit OS components and creates a Windows installer package.
    It requires NSIS to be installed for building the installer.
.NOTES
    Version: 1.0
    Author: TernaryBit OS Team
    Prerequisites: 
    - Visual Studio Build Tools with C++ workload
    - NSIS (Nullsoft Scriptable Install System)
    - 7-Zip (for creating ZIP archives)
#>

#Requires -RunAsAdministrator

# Configuration
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build\windows"
$BinDir = Join-Path $BuildDir "bin"
$DistDir = Join-Path $ProjectRoot "dist"
$InstallerScript = Join-Path $PSScriptRoot "installer.nsi"
$Version = "1.0.0"

# Create directories if they don't exist
$null = New-Item -ItemType Directory -Force -Path $BinDir
$null = New-Item -ItemType Directory -Force -Path $DistDir

# Step 1: Build the project
Write-Host "🚀 Building TernaryBit OS..." -ForegroundColor Cyan
& "$PSScriptRoot\build-tbos-windows.bat"
if ($LASTEXITCODE -ne 0) {
    Write-Error "❌ Build failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

# Step 2: Check for NSIS
$nsisPath = "C:\Program Files (x86)\NSIS\makensis.exe"
if (-not (Test-Path $nsisPath)) {
    $nsisPath = "C:\Program Files\NSIS\makensis.exe"
    if (-not (Test-Path $nsisPath)) {
        Write-Error "❌ NSIS not found. Please install NSIS from https://nsis.sourceforge.io/Download"
        exit 1
    }
}

# Step 3: Build the installer
Write-Host "📦 Creating installer..." -ForegroundColor Cyan
& $nsisPath "/V3" "/DVersion=$Version" "$InstallerScript"
if ($LASTEXITCODE -ne 0) {
    Write-Error "❌ Installer creation failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

# Step 4: Create ZIP package
$zipFile = Join-Path $DistDir "TernaryBitOS-Windows-$Version.zip"
Write-Host "📦 Creating ZIP package: $zipFile" -ForegroundColor Cyan
if (Get-Command Compress-Archive -ErrorAction SilentlyContinue) {
    # PowerShell 5.0+
    Compress-Archive -Path "$BinDir\*" -DestinationPath $zipFile -Force
} else {
    # Fallback to 7-Zip if available
    $7zip = "C:\Program Files\7-Zip\7z.exe"
    if (Test-Path $7zip) {
        & $7zip a -tzip $zipFile "$BinDir\*"
    } else {
        Write-Warning "Could not create ZIP archive. Install 7-Zip or use PowerShell 5.0+"
    }
}

# Step 5: Generate checksums
Write-Host "🔒 Generating checksums..." -ForegroundColor Cyan
$installerExe = Join-Path $BuildDir "TernaryBitOS-Installer.exe"
$checksumFile = Join-Path $DistDir "checksums.sha256"

if (Test-Path $installerExe) {
    $installerHash = (Get-FileHash -Path $installerExe -Algorithm SHA256).Hash
    "$installerHash  $(Split-Path $installerExe -Leaf)" | Out-File -FilePath $checksumFile -Encoding ASCII
}

if (Test-Path $zipFile) {
    $zipHash = (Get-FileHash -Path $zipFile -Algorithm SHA256).Hash
    "$zipHash  $(Split-Path $zipFile -Leaf)" | Out-File -FilePath $checksumFile -Encoding ASCII -Append
}

Write-Host "✅ Build and packaging completed successfully!" -ForegroundColor Green
Write-Host "📦 Installer: $installerExe"
Write-Host "📦 ZIP Package: $zipFile"
Write-Host "🔒 Checksums: $checksumFile"
