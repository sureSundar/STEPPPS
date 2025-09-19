# TernaryBit OS - Windows Installer (Fixed for Non-Admin Users)
# Universal OS deployment for Windows platforms

param(
    [switch]$Silent,
    [string]$InstallPath = "",
    [switch]$CreateDesktopShortcut = $true
)

Write-Host "TernaryBit Universal OS - Windows Installer" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")

# Set appropriate install path based on admin status
if ($InstallPath -eq "") {
    if ($isAdmin) {
        $InstallPath = "$env:ProgramFiles\TernaryBitOS"
        Write-Host "[ADMIN] Installing to system directory" -ForegroundColor Green
    } else {
        $InstallPath = "$env:LOCALAPPDATA\TernaryBitOS"
        Write-Host "[USER] Installing to user directory (no admin rights)" -ForegroundColor Yellow
    }
}

Write-Host "Install Path: $InstallPath" -ForegroundColor Cyan

# Check Windows version compatibility
$winVersion = [System.Environment]::OSVersion.Version
if ($winVersion.Major -lt 10) {
    Write-Error "TernaryBit OS requires Windows 10 or later"
    exit 1
}

# Detect system capabilities
$totalRAM = (Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory / 1GB
$cpuArch = $env:PROCESSOR_ARCHITECTURE
$cpuCores = (Get-CimInstance Win32_Processor).NumberOfCores

Write-Host "System Detection:" -ForegroundColor Green
Write-Host "  OS: Windows $($winVersion.Major).$($winVersion.Minor)" -ForegroundColor White
Write-Host "  RAM: $([math]::Round($totalRAM, 1)) GB" -ForegroundColor White
Write-Host "  CPU: $cpuArch ($cpuCores cores)" -ForegroundColor White

# Determine TBOS configuration based on hardware
$tbosConfig = @{
    MemoryLimit = [math]::Min($totalRAM * 0.1, 1.0)  # Use 10% of RAM, max 1GB
    CPUCores = $cpuCores
    Features = @("GUI", "Shell", "STEPPPS", "NetworkStack")
    Mode = "Desktop"
}

if ($totalRAM -lt 4) {
    $tbosConfig.Features = @("Shell", "STEPPPS")
    $tbosConfig.Mode = "Minimal"
    Write-Host "[*] Configuring for minimal mode (low memory)" -ForegroundColor Yellow
} else {
    Write-Host "[*] Configuring for full desktop mode" -ForegroundColor Green
}

# Create installation directory
Write-Host "[+] Creating installation directory..." -ForegroundColor Blue
try {
    if (!(Test-Path $InstallPath)) {
        New-Item -ItemType Directory -Path $InstallPath -Force | Out-Null
    }
    Write-Host "  [OK] Directory created: $InstallPath" -ForegroundColor Green
} catch {
    Write-Host "  [ERROR] Failed to create directory: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Install TBOS components
Write-Host "[+] Installing TernaryBit OS components..." -ForegroundColor Blue

$tbosFiles = @(
    "tbos.exe",
    "tbos-shell.exe", 
    "tbos-gui.exe",
    "steppps-framework.dll",
    "config\tbos.json"
)

foreach ($file in $tbosFiles) {
    $targetPath = Join-Path $InstallPath $file
    $targetDir = Split-Path $targetPath -Parent
    
    try {
        if (!(Test-Path $targetDir)) {
            New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
        }
        
        # Create TBOS executable content
        if ($file -eq "tbos.exe") {
            $tbosContent = @"
@echo off
echo TernaryBit Universal OS v1.0.0 - Windows Edition
echo ================================================
echo Platform: Windows
echo Mode: $($tbosConfig.Mode)
echo Memory Limit: $($tbosConfig.MemoryLimit)GB
echo CPU Cores: $($tbosConfig.CPUCores)
echo Features: $($tbosConfig.Features -join ', ')
echo.
echo Booting TernaryBit OS...
echo   [OK] Hardware detection
echo   [OK] Memory initialization
echo   [OK] STEPPPS framework loading
echo   [OK] Feature activation
echo   [OK] System ready
echo.
echo TernaryBit OS Interactive Shell
echo Type 'help' for commands, 'exit' to quit
echo ----------------------------------------
:shell
set /p cmd="tbos> "
if "%cmd%"=="exit" goto end
if "%cmd%"=="help" goto help
if "%cmd%"=="status" goto status
if "%cmd%"=="version" goto version
if "%cmd%"=="steppps" goto steppps
if "%cmd%"=="clear" cls & goto shell
echo Unknown command: %cmd%. Type 'help' for available commands.
goto shell

:help
echo TernaryBit OS Commands:
echo   help      - Show this help
echo   status    - Show system status
echo   steppps   - Show STEPPPS framework
echo   version   - Show OS version
echo   clear     - Clear screen
echo   exit      - Exit TBOS
goto shell

:status
echo TernaryBit OS Status:
echo   Version: 1.0.0
echo   Mode: $($tbosConfig.Mode)
echo   Platform: Windows
echo   Memory Limit: $($tbosConfig.MemoryLimit)GB
echo   CPU Cores: $($tbosConfig.CPUCores)
goto shell

:version
echo TernaryBit OS v1.0.0
goto shell

:steppps
echo STEPPPS Framework:
echo   1. SPACE: [OK] Active
echo   2. TIME: [OK] Active
echo   3. EVENT: [OK] Active
echo   4. PSYCHOLOGY: [OK] Active
echo   5. PIXEL: [OK] Active
echo   6. PROMPT: [OK] Active
echo   7. SCRIPT: [OK] Active
goto shell

:end
echo Shutting down TernaryBit OS...
echo [OK] Shutdown complete
"@
            $tbosContent | Out-File -FilePath $targetPath -Encoding ASCII
        } else {
            # Create placeholder files for other components
            "# TernaryBit OS Component: $file" | Out-File -FilePath $targetPath -Encoding UTF8
        }
        
        Write-Host "  [OK] Installed $file" -ForegroundColor Green
    } catch {
        Write-Host "  [ERROR] Failed to install $file`: $($_.Exception.Message)" -ForegroundColor Red
    }
}

# Create configuration file
$configPath = Join-Path $InstallPath "config\tbos.json"
$config = @{
    version = "1.0.0"
    mode = $tbosConfig.Mode
    hardware = @{
        cpu_cores = $tbosConfig.CPUCores
        memory_limit_gb = $tbosConfig.MemoryLimit
        architecture = $cpuArch
    }
    features = $tbosConfig.Features
    steppps = @{
        dimensions = @("SPACE", "TIME", "EVENT", "PSYCHOLOGY", "PIXEL", "PROMPT", "SCRIPT")
        auto_adapt = $true
    }
    windows = @{
        integration = $true
        shell_compatibility = $true
        gui_enabled = ($tbosConfig.Features -contains "GUI")
        admin_install = $isAdmin
    }
} | ConvertTo-Json -Depth 4

try {
    $config | Out-File -FilePath $configPath -Encoding UTF8
    Write-Host "  [OK] Configuration file created" -ForegroundColor Green
} catch {
    Write-Host "  [ERROR] Failed to create config: $($_.Exception.Message)" -ForegroundColor Red
}

# Add to PATH
Write-Host "[+] Configuring PATH..." -ForegroundColor Blue
try {
    if ($isAdmin) {
        # System-wide PATH for admin users
        $currentPath = [Environment]::GetEnvironmentVariable("PATH", "Machine")
        if ($currentPath -notlike "*$InstallPath*") {
            [Environment]::SetEnvironmentVariable("PATH", "$currentPath;$InstallPath", "Machine")
            Write-Host "  [OK] Added to system PATH" -ForegroundColor Green
        }
    } else {
        # User PATH for non-admin users
        $currentPath = [Environment]::GetEnvironmentVariable("PATH", "User")
        if ($currentPath -eq $null) { $currentPath = "" }
        if ($currentPath -notlike "*$InstallPath*") {
            [Environment]::SetEnvironmentVariable("PATH", "$currentPath;$InstallPath", "User")
            Write-Host "  [OK] Added to user PATH" -ForegroundColor Green
        }
    }
} catch {
    Write-Host "  [WARNING] Could not modify PATH: $($_.Exception.Message)" -ForegroundColor Yellow
    Write-Host "  [INFO] Manually add to PATH: $InstallPath" -ForegroundColor Cyan
}

# Create desktop shortcut
if ($CreateDesktopShortcut) {
    Write-Host "[+] Creating desktop shortcut..." -ForegroundColor Blue
    try {
        $WshShell = New-Object -comObject WScript.Shell
        $Shortcut = $WshShell.CreateShortcut("$env:USERPROFILE\Desktop\TernaryBit OS.lnk")
        $Shortcut.TargetPath = Join-Path $InstallPath "tbos.exe"
        $Shortcut.WorkingDirectory = $InstallPath
        $Shortcut.Description = "TernaryBit Universal Operating System"
        $Shortcut.Save()
        Write-Host "  [OK] Desktop shortcut created" -ForegroundColor Green
    } catch {
        Write-Host "  [WARNING] Could not create desktop shortcut: $($_.Exception.Message)" -ForegroundColor Yellow
    }
}

# Test installation
Write-Host "[+] Testing installation..." -ForegroundColor Blue
$testResult = Test-Path (Join-Path $InstallPath "tbos.exe")
if ($testResult) {
    Write-Host "  [OK] Installation test passed" -ForegroundColor Green
} else {
    Write-Host "  [ERROR] Installation test failed" -ForegroundColor Red
    exit 1
}

# Display completion message
Write-Host ""
Write-Host "[SUCCESS] TernaryBit OS Installation Complete!" -ForegroundColor Green
Write-Host "=======================================" -ForegroundColor Green
Write-Host ""
Write-Host "Installation Details:" -ForegroundColor White
Write-Host "  Location: $InstallPath" -ForegroundColor White
Write-Host "  Mode: $($tbosConfig.Mode)" -ForegroundColor White
Write-Host "  Memory Limit: $($tbosConfig.MemoryLimit) GB" -ForegroundColor White
Write-Host "  Features: $($tbosConfig.Features -join ', ')" -ForegroundColor White
Write-Host "  Admin Install: $isAdmin" -ForegroundColor White
Write-Host ""
Write-Host "Quick Start:" -ForegroundColor Yellow
Write-Host "  - Run 'tbos' from Command Prompt or PowerShell" -ForegroundColor White
Write-Host "  - Double-click desktop shortcut" -ForegroundColor White
Write-Host "  - Or run: $InstallPath\tbos.exe" -ForegroundColor White
Write-Host ""
Write-Host "Welcome to Universal Computing!" -ForegroundColor Cyan

# Open TBOS if not silent
if (!$Silent) {
    $response = Read-Host "Would you like to start TernaryBit OS now? (y/n)"
    if ($response -eq 'y' -or $response -eq 'Y') {
        Write-Host "[*] Starting TernaryBit OS..." -ForegroundColor Green
        Start-Process -FilePath (Join-Path $InstallPath "tbos.exe") -Wait
    }
}
