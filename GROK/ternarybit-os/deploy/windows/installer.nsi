; TernaryBit OS Windows Installer
; NSIS Script for creating Windows installation package

; Include Modern UI
!include "MUI2.nsh"

; General settings
Name "TernaryBit OS"
OutFile "..\..\build\windows\TernaryBitOS-Installer.exe"
InstallDir "$PROGRAMFILES\TernaryBitOS"
InstallDirRegKey HKLM "Software\TernaryBitOS" "Install_Dir"

; Request application privileges for Windows Vista and later
RequestExecutionLevel admin

; Variables
Var StartMenuFolder

; Interface Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Languages
!insertmacro MUI_LANGUAGE "English"

; Installer sections
Section "MainSection" SecMain
  SectionIn RO
  
  SetOutPath "$INSTDIR"
  
  ; Install files
  File /r "..\..\build\windows\bin\*.*"
  
  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  ; Add to Add/Remove Programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TernaryBitOS" \
                 "DisplayName" "TernaryBit OS"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TernaryBitOS" \
                 "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TernaryBitOS" \
                   "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TernaryBitOS" \
                   "NoRepair" 1
  
  ; Create start menu shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\TernaryBit OS.lnk" "$INSTDIR\tbos_kernel.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END
  
  ; Add to PATH
  EnVar::SetHKCU
  EnVar::AddValue "PATH" "$INSTDIR"
  
  ; Create desktop shortcut
  CreateShortCut "$DESKTOP\TernaryBit OS.lnk" "$INSTDIR\tbos_kernel.exe"
  
SectionEnd

; Uninstaller section
Section "Uninstall"
  ; Remove files
  RMDir /r "$INSTDIR"
  
  ; Remove start menu shortcuts
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  RMDir /r "$SMPROGRAMS\$StartMenuFolder"
  
  ; Remove desktop shortcut
  Delete "$DESKTOP\TernaryBit OS.lnk"
  
  ; Remove from PATH
  EnVar::SetHKCU
  EnVar::DeleteValue "PATH" "$INSTDIR"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TernaryBitOS"
  DeleteRegKey /ifempty HKLM "Software\TernaryBitOS"
  
SectionEnd

; Functions
Function .onInit
  ; Check for admin rights
  UserInfo::GetAccountType
  pop $0
  ${If} $0 != "admin"
    MessageBox mb_iconstop "Administrator rights required to install TernaryBit OS"
    SetErrorLevel 740 ; ERROR_ELEVATION_REQUIRED
    Quit
  ${EndIf}
  
  ; Check for required Windows version (Windows 7 or later)
  ${IfNot} ${AtLeastWin7}
    MessageBox mb_iconstop "Windows 7 or later is required to install TernaryBit OS"
    Quit
  ${EndIf}
FunctionEnd
