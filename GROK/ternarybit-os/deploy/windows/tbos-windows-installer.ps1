# TernaryBit OS - Windows 11 Installer
# Universal OS deployment for Windows platforms

param(
    [switch]$Silent,
    [string]$InstallPath = "$env:ProgramFiles\TernaryBitOS",
    [switch]$CreateDesktopShortcut = $true
)

Write-Host "TernaryBit Universal OS - Windows Installer" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan

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

# Check if running as administrator and adjust install path if needed
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")

if (-not $isAdmin) {
    $InstallPath = "$env:LOCALAPPDATA\TernaryBitOS"
    Write-Host "[!] Not running as administrator, installing to user directory" -ForegroundColor Yellow
    Write-Host "    Install path: $InstallPath" -ForegroundColor Yellow
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

# Copy TBOS files (simulated - in real deployment these would be actual binaries)
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
        
        # Simulate file creation (in real deployment, copy from source)
        "# TernaryBit OS Component: $file" | Out-File -FilePath $targetPath -Encoding UTF8
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
    }
} | ConvertTo-Json -Depth 4

try {
    $config | Out-File -FilePath $configPath -Encoding UTF8
    Write-Host "  [OK] Configuration file created" -ForegroundColor Green
} catch {
    Write-Host "  [ERROR] Failed to create config: $($_.Exception.Message)" -ForegroundColor Red
}

# Add to PATH
Write-Host "[+] Configuring system PATH..." -ForegroundColor Blue
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
        if ($currentPath -notlike "*$InstallPath*") {
            [Environment]::SetEnvironmentVariable("PATH", "$currentPath;$InstallPath", "User")
            Write-Host "  [OK] Added to user PATH" -ForegroundColor Green
        }
    }
} catch {
    Write-Host "  [WARNING] Could not modify PATH: $($_.Exception.Message)" -ForegroundColor Yellow
    Write-Host "  [INFO] You can manually add: $InstallPath" -ForegroundColor Cyan
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

# Create Windows service (optional)
Write-Host "[+] Configuring TBOS service..." -ForegroundColor Blue
$serviceScript = @"
# TernaryBit OS Service
`$serviceName = "TernaryBitOS"
`$servicePath = "$InstallPath\tbos.exe --service"

if (Get-Service `$serviceName -ErrorAction SilentlyContinue) {
    Stop-Service `$serviceName -Force
    Remove-Service `$serviceName
}

New-Service -Name `$serviceName -BinaryPathName `$servicePath -DisplayName "TernaryBit Universal OS" -Description "Universal Operating System Framework" -StartupType Manual
"@

try {
    $serviceScript | Out-File -FilePath (Join-Path $InstallPath "install-service.ps1") -Encoding UTF8
    Write-Host "  [OK] Service script created" -ForegroundColor Green
} catch {
    Write-Host "  [WARNING] Could not create service script: $($_.Exception.Message)" -ForegroundColor Yellow
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
Write-Host ""
Write-Host "Quick Start:" -ForegroundColor Yellow
Write-Host "  - Run 'tbos' from Command Prompt or PowerShell" -ForegroundColor White
Write-Host "  - Double-click desktop shortcut" -ForegroundColor White
Write-Host "  - Use 'tbos --help' for command options" -ForegroundColor White
Write-Host ""
Write-Host "Welcome to Universal Computing!" -ForegroundColor Cyan

# Open TBOS if not silent
if (!$Silent) {
    $response = Read-Host "Would you like to start TernaryBit OS now? (y/n)"
    if ($response -eq 'y' -or $response -eq 'Y') {
        Write-Host "[*] Starting TernaryBit OS..." -ForegroundColor Green
        # In real deployment: Start-Process (Join-Path $InstallPath "tbos.exe")
        Write-Host "TernaryBit OS would start here (simulated for demo)" -ForegroundColor Yellow
    }
}
