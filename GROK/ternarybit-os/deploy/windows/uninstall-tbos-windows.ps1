# TernaryBit OS - Windows Uninstaller
# Universal OS uninstallation for Windows platforms

param(
    [switch]$Silent,
    [string]$InstallPath = ""
)

Write-Host "TernaryBit Universal OS - Windows Uninstaller" -ForegroundColor Cyan
Write-Host "==============================================" -ForegroundColor Cyan

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")

# Default installation paths
$programFilesPath = "$env:ProgramFiles\TernaryBitOS"
$localAppDataPath = "$env:LOCALAPPDATA\TernaryBitOS"

# Determine installation path
if ($InstallPath -eq "") {
    # Check both possible installation locations
    if (Test-Path $programFilesPath) {
        $InstallPath = $programFilesPath
    } elseif (Test-Path $localAppDataPath) {
        $InstallPath = $localAppDataPath
    } else {
        Write-Error "TernaryBit OS installation not found in default locations."
        Write-Host "Please specify the installation path using -InstallPath parameter." -ForegroundColor Yellow
        exit 1
    }
} elseif (-not (Test-Path $InstallPath)) {
    Write-Error "Specified installation path does not exist: $InstallPath"
    exit 1
}

# Confirm uninstallation
if (-not $Silent) {
    Write-Host "This will uninstall TernaryBit OS from: $InstallPath" -ForegroundColor Yellow
    $confirmation = Read-Host "Are you sure you want to continue? (y/n)"
    if ($confirmation -ne 'y' -and $confirmation -ne 'Y') {
        Write-Host "Uninstallation cancelled." -ForegroundColor Yellow
        exit 0
    }
}

# Stop any running TernaryBit OS processes
Write-Host "[1/5] Stopping running processes..." -ForegroundColor Cyan
Get-Process | Where-Object { $_.ProcessName -like "*tbos*" -or $_.ProcessName -like "*ternarybit*" } | Stop-Process -Force -ErrorAction SilentlyContinue

# Remove shortcuts
Write-Host "[2/5] Removing shortcuts..." -ForegroundColor Cyan
$desktopShortcut = [System.Environment]::GetFolderPath('Desktop') + "\TernaryBit OS.lnk"
$startMenuShortcut = [System.Environment]::GetFolderPath('StartMenu') + "\Programs\TernaryBit OS\TernaryBit OS.lnk"
$startMenuDir = [System.Environment]::GetFolderPath('StartMenu') + "\Programs\TernaryBit OS"

if (Test-Path $desktopShortcut) { Remove-Item $desktopShortcut -Force }
if (Test-Path $startMenuShortcut) { Remove-Item $startMenuShortcut -Force }
if (Test-Path $startMenuDir) { Remove-Item $startMenuDir -Recurse -Force }

# Remove registry entries (if installed for all users)
if ($isAdmin) {
    Write-Host "[3/5] Removing registry entries..." -ForegroundColor Cyan
    $uninstallKey = "HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\TernaryBitOS"
    if (Test-Path $uninstallKey) {
        Remove-Item -Path $uninstallKey -Recurse -Force
    }
    
    # Remove from PATH if added
    $currentPath = [Environment]::GetEnvironmentVariable('Path', 'Machine')
    if ($currentPath -like "*$InstallPath*") {
        $newPath = ($currentPath -split ';' | Where-Object { $_ -ne $InstallPath -and $_ -ne "$InstallPath\bin" }) -join ';'
        [Environment]::SetEnvironmentVariable('Path', $newPath, 'Machine')
    }
}

# Remove installation directory
Write-Host "[4/5] Removing installation directory..." -ForegroundColor Cyan
if (Test-Path $InstallPath) {
    try {
        Remove-Item -Path $InstallPath -Recurse -Force -ErrorAction Stop
        Write-Host "  Removed: $InstallPath" -ForegroundColor Green
    } catch {
        Write-Host "  Warning: Could not remove $InstallPath. Some files may be in use." -ForegroundColor Yellow
        Write-Host "  You may need to manually remove the directory after restarting your computer." -ForegroundColor Yellow
    }
}

# Clean up temporary files
Write-Host "[5/5] Cleaning up temporary files..." -ForegroundColor Cyan
$tempDir = [System.IO.Path]::GetTempPath()
Get-ChildItem -Path $tempDir -Filter "tbos-*" -Directory | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue

Write-Host "`nUninstallation complete!" -ForegroundColor Green
Write-Host "TernaryBit OS has been successfully uninstalled from your system." -ForegroundColor Green

if (-not $Silent) {
    Read-Host "Press Enter to exit"
}

exit 0
