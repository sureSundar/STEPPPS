; TBOS Universal Detector - NSIS Installer Script
; 🕉️ Swamiye Saranam Aiyappa 🕉️
;
; This creates a Windows installer (.exe) for TBOS Detector
; Use: makensis installer_windows.nsi

;--------------------------------
; Modern UI

!include "MUI2.nsh"

;--------------------------------
; General

  ; Name and file
  Name "TBOS Universal Detector"
  OutFile "dist/windows/TBOS_Detector_Setup_v1.0.0.exe"

  ; Default installation folder
  InstallDir "$PROGRAMFILES64\TBOS\Detector"

  ; Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\TBOS\Detector" "InstallDir"

  ; Request application privileges
  RequestExecutionLevel admin

  ; Compression
  SetCompressor /SOLID lzma

  ; Version Info
  VIProductVersion "1.0.0.0"
  VIAddVersionKey "ProductName" "TBOS Universal Detector"
  VIAddVersionKey "CompanyName" "TBOS Project"
  VIAddVersionKey "LegalCopyright" "© 2025 TBOS Project"
  VIAddVersionKey "FileDescription" "Universal Hardware Detection Tool"
  VIAddVersionKey "FileVersion" "1.0.0.0"

;--------------------------------
; Variables

  Var StartMenuFolder

;--------------------------------
; Interface Settings

  !define MUI_ABORTWARNING
  !define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
  !define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

  ; Welcome page
  !define MUI_WELCOMEPAGE_TITLE "TBOS Universal Hardware Detector"
  !define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of TBOS Universal Detector.$\r$\n$\r$\n🕉 Swamiye Saranam Aiyappa 🕉$\r$\n$\r$\nTBOS Detector identifies your hardware and recommends the optimal operating system for your device.$\r$\n$\r$\nClick Next to continue."

;--------------------------------
; Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
  !insertmacro MUI_PAGE_DIRECTORY

  ; Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\TBOS\Detector"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  !insertmacro MUI_PAGE_INSTFILES

  !define MUI_FINISHPAGE_NOAUTOCLOSE
  !define MUI_FINISHPAGE_RUN "$INSTDIR\tbos_detector_x64.exe"
  !define MUI_FINISHPAGE_RUN_TEXT "Run TBOS Detector now"
  !define MUI_FINISHPAGE_LINK "Visit TBOS Project Website"
  !define MUI_FINISHPAGE_LINK_LOCATION "https://github.com/TBOS"

  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
; Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Installer Sections

Section "TBOS Detector (required)" SecMain

  SectionIn RO

  ; Set output path to the installation directory
  SetOutPath "$INSTDIR"

  ; Put files there
  File "build\windows\tbos_detector_x64.exe"
  File "build\windows\tbos_detector_x86.exe"
  File "dist\windows\README_WINDOWS.txt"
  File "dist\windows\RUN_DETECTOR.bat"
  File "dist\windows\GET_JSON.bat"

  ; Store installation folder
  WriteRegStr HKLM "Software\TBOS\Detector" "InstallDir" $INSTDIR

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Note: PATH modification removed (requires additional plugin)
  ; Users can run from Start Menu shortcuts or use full path

  ; Create Start Menu shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\TBOS Detector.lnk" "$INSTDIR\RUN_DETECTOR.bat" "" "$INSTDIR\tbos_detector_x64.exe" 0
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\README.lnk" "$INSTDIR\README_WINDOWS.txt"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Command Prompt Here.lnk" "cmd.exe" "/k cd /d $INSTDIR"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_END

  ; Create Desktop shortcut (optional)
  CreateShortcut "$DESKTOP\TBOS Detector.lnk" "$INSTDIR\RUN_DETECTOR.bat" "" "$INSTDIR\tbos_detector_x64.exe" 0

  ; Write uninstall keys
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TBOS_Detector" "DisplayName" "TBOS Universal Detector"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TBOS_Detector" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TBOS_Detector" "DisplayIcon" "$INSTDIR\tbos_detector_x64.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TBOS_Detector" "Publisher" "TBOS Project"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TBOS_Detector" "DisplayVersion" "1.0.0"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TBOS_Detector" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TBOS_Detector" "NoRepair" 1

SectionEnd

;--------------------------------
; Uninstaller Section

Section "Uninstall"

  ; Remove files
  Delete "$INSTDIR\tbos_detector_x64.exe"
  Delete "$INSTDIR\tbos_detector_x86.exe"
  Delete "$INSTDIR\README_WINDOWS.txt"
  Delete "$INSTDIR\RUN_DETECTOR.bat"
  Delete "$INSTDIR\GET_JSON.bat"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\hardware_profile.json"

  ; Remove directories
  RMDir "$INSTDIR"

  ; Remove Start Menu folder
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$SMPROGRAMS\$StartMenuFolder\TBOS Detector.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\README.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Command Prompt Here.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  ; Remove Desktop shortcut
  Delete "$DESKTOP\TBOS Detector.lnk"

  ; Note: PATH modification was not added during install

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TBOS_Detector"
  DeleteRegKey HKLM "Software\TBOS\Detector"
  DeleteRegKey /ifempty HKLM "Software\TBOS"

SectionEnd

;--------------------------------
; Descriptions

  ; Language strings
  LangString DESC_SecMain ${LANG_ENGLISH} "TBOS Universal Hardware Detector - Cross-platform hardware detection tool"

  ; Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
