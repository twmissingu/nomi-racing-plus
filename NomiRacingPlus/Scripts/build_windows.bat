@echo off
REM ==============================================================================
REM NIO Racing Plus - Windows Build, Sign, Package, and Distribute
REM ==============================================================================
REM Usage:
REM   build_windows.bat [command] [options]
REM
REM Commands:
REM   build       - Build the UE5 project for Windows
REM   sign        - Code sign an existing build
REM   package     - Create installer (NSIS) or ZIP archive
REM   release     - Full pipeline: build -> sign -> package
REM   clean       - Remove build artifacts
REM   help        - Show this help message
REM
REM Options:
REM   --build-type [Development|Shipping|Debug]  (default: Shipping)
REM   --ue5 PATH                                 UE5 installation path
REM   --skip-sign                                Skip code signing
REM   --skip-package                             Skip installer creation
REM   --installer [nsis|inno|zip]               Installer framework (default: nsis)
REM   --verbose                                  Enable verbose output
REM
REM Environment Variables:
REM   UE5_ROOT             - Path to UE5 installation
REM   WIN_SIGNING_CERT     - Path to PFX certificate file
REM   WIN_SIGNING_PASSWORD - Certificate password
REM   WIN_TIMESTAMP_SERVER - Timestamp server URL
REM ==============================================================================

setlocal enabledelayedexpansion

REM --- Configuration ---
set "PROJECT_NAME=NomiRacingPlus"
set "PROJECT_VERSION=1.0.0"
set "PROJECT_DISPLAY_NAME=NIO Racing Plus"
set "PROJECT_COMPANY=NIO Racing Plus"
set "DEFAULT_BUILD_TYPE=Shipping"
set "BUILD_DIR=Build"
set "ARCHIVE_DIR=%BUILD_DIR%\Archive"
set "PACKAGES_DIR=%BUILD_DIR%\Packages"
set "LOGS_DIR=%BUILD_DIR%\Logs"

REM --- Defaults ---
set "COMMAND=%~1"
if "%COMMAND%"=="" set "COMMAND=help"
set "BUILD_TYPE=%DEFAULT_BUILD_TYPE%"
set "UE5_DIR="
set "SKIP_SIGN=false"
set "SKIP_PACKAGE=false"
set "VERBOSE=false"
set "INSTALLER_FRAMEWORK=nsis"

REM --- Parse Arguments ---
:parse_args
shift
if "%~1"=="" goto :args_done
if "%~1"=="--build-type" (
    set "BUILD_TYPE=%~2"
    shift
    shift
    goto :parse_args
)
if "%~1"=="--ue5" (
    set "UE5_DIR=%~2"
    shift
    shift
    goto :parse_args
)
if "%~1"=="--skip-sign" (
    set "SKIP_SIGN=true"
    shift
    goto :parse_args
)
if "%~1"=="--skip-package" (
    set "SKIP_PACKAGE=true"
    shift
    goto :parse_args
)
if "%~1"=="--installer" (
    set "INSTALLER_FRAMEWORK=%~2"
    shift
    shift
    goto :parse_args
)
if "%~1"=="--verbose" (
    set "VERBOSE=true"
    shift
    goto :parse_args
)
echo [WARN] Unknown option: %~1
shift
goto :parse_args

:args_done

REM --- Route Commands ---
if /i "%COMMAND%"=="build" goto :cmd_build
if /i "%COMMAND%"=="sign" goto :cmd_sign
if /i "%COMMAND%"=="package" goto :cmd_package
if /i "%COMMAND%"=="release" goto :cmd_release
if /i "%COMMAND%"=="clean" goto :cmd_clean
if /i "%COMMAND%"=="verify" goto :cmd_verify
if /i "%COMMAND%"=="help" goto :cmd_help
if /i "%COMMAND%"=="--help" goto :cmd_help
if /i "%COMMAND%"=="-h" goto :cmd_help
echo [ERROR] Unknown command: %COMMAND%
goto :cmd_help

REM ==============================================================================
REM HELP
REM ==============================================================================
:cmd_help
echo.
echo NIO Racing Plus - Windows Build Automation
echo ===========================================
echo.
echo Usage: build_windows.bat [command] [options]
echo.
echo Commands:
echo   build       Build the UE5 project for Windows
echo   sign        Code sign an existing build
echo   package     Create installer or ZIP archive
echo   release     Full pipeline: build -^> sign -^> package
echo   verify      Verify code signatures
echo   clean       Remove build artifacts
echo   help        Show this help message
echo.
echo Options:
echo   --build-type [Development^|Shipping^|Debug]  Build config (default: Shipping)
echo   --ue5 PATH                                 UE5 installation path
echo   --skip-sign                                Skip code signing
echo   --skip-package                             Skip installer creation
echo   --installer [nsis^|inno^|zip]               Installer type (default: nsis)
echo   --verbose                                  Enable verbose output
echo.
echo Environment Variables:
echo   UE5_ROOT             Path to UE5 installation
echo   WIN_SIGNING_CERT     Path to PFX certificate
echo   WIN_SIGNING_PASSWORD Certificate password
echo   WIN_TIMESTAMP_SERVER Timestamp server URL
echo.
echo Examples:
echo   build_windows.bat build
echo   build_windows.bat release --skip-sign
echo   build_windows.bat package --installer zip
echo   build_windows.bat sign
echo.
goto :eof

REM ==============================================================================
REM FIND UE5
REM ==============================================================================
:find_ue5
if defined UE5_DIR (
    if exist "%UE5_DIR%" (
        echo [INFO] Using UE5 from: %UE5_DIR%
        goto :eof
    )
)

if defined UE5_ROOT (
    if exist "%UE5_ROOT%" (
        set "UE5_DIR=%UE5_ROOT%"
        echo [INFO] Using UE5 from UE5_ROOT: %UE5_DIR%
        goto :eof
    )
)

REM Common Windows locations
for %%V in (5.5 5.4 5.3) do (
    if exist "C:\Program Files\Epic Games\UE_%%V" (
        set "UE5_DIR=C:\Program Files\Epic Games\UE_%%V"
        echo [INFO] Found UE5 at: !UE5_DIR!
        goto :eof
    )
    if exist "D:\Program Files\Epic Games\UE_%%V" (
        set "UE5_DIR=D:\Program Files\Epic Games\UE_%%V"
        echo [INFO] Found UE5 at: !UE5_DIR!
        goto :eof
    )
)

echo [ERROR] UE5 not found. Set UE5_ROOT or use --ue5 PATH
exit /b 1

REM ==============================================================================
REM INIT DIRECTORIES
REM ==============================================================================
:init_dirs
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%ARCHIVE_DIR%" mkdir "%ARCHIVE_DIR%"
if not exist "%PACKAGES_DIR%" mkdir "%PACKAGES_DIR%"
if not exist "%LOGS_DIR%" mkdir "%LOGS_DIR%"
goto :eof

REM ==============================================================================
REM FIND SIGNTOOL
REM ==============================================================================
:find_signtool
if defined WIN_SIGNTOOL_PATH (
    if exist "%WIN_SIGNTOOL_PATH%" goto :eof
)

REM Search Windows SDK locations
for %%V in (10.0.26100.0 10.0.22621.0 10.0.22000.0 10.0.20348.0 10.0.19041.0 10.0.18362.0) do (
    if exist "C:\Program Files (x86)\Windows Kits\10\bin\%%V\x64\signtool.exe" (
        set "WIN_SIGNTOOL_PATH=C:\Program Files (x86)\Windows Kits\10\bin\%%V\x64\signtool.exe"
        echo [INFO] Found signtool at: !WIN_SIGNTOOL_PATH!
        goto :eof
    )
)

REM Fallback: search all SDK versions
for /f "delims=" %%F in ('dir /b /s "C:\Program Files (x86)\Windows Kits\10\bin\*\x64\signtool.exe" 2^>nul') do (
    set "WIN_SIGNTOOL_PATH=%%F"
    echo [INFO] Found signtool at: !WIN_SIGNTOOL_PATH!
    goto :eof
)

echo [ERROR] signtool.exe not found. Install Windows SDK or set WIN_SIGNTOOL_PATH
exit /b 1

REM ==============================================================================
REM BUILD
REM ==============================================================================
:cmd_build
echo [STEP] Building %PROJECT_DISPLAY_NAME% for Windows (%BUILD_TYPE%)...
echo.

call :find_ue5
if errorlevel 1 exit /b 1
call :init_dirs

set "UAT_PATH=%UE5_DIR%\Engine\Build\BatchFiles\RunUAT.bat"
if not exist "%UAT_PATH%" (
    echo [ERROR] RunUAT.bat not found at: %UAT_PATH%
    exit /b 1
)

set "LOG_FILE=%LOGS_DIR%\build_windows_%BUILD_TYPE%_%DATE:~10,4%%DATE:~4,2%%DATE:~7,2%_%TIME:~0,2%%TIME:~3,2%%TIME:~6,2%.log"
set "LOG_FILE=%LOG_FILE: =0%"

echo [INFO] Running UAT: %UAT_PATH%
echo [INFO] Log: %LOG_FILE%
echo.

call "%UAT_PATH%" ^
    BuildCookRun ^
    -project="%~dp0..\%PROJECT_NAME%.uproject" ^
    -noP4 ^
    -platform=Win64 ^
    -clientconfig=%BUILD_TYPE% ^
    -cook ^
    -build ^
    -stage ^
    -package ^
    -archive ^
    -archivedirectory="%ARCHIVE_DIR%" ^
    -pak ^
    -prereqs ^
    -utf8output ^
    > "%LOG_FILE%" 2>&1

if errorlevel 1 (
    echo [ERROR] Build failed with exit code: %ERRORLEVEL%
    echo [ERROR] See log: %LOG_FILE%
    exit /b 1
)

echo [INFO] Build completed successfully
echo [INFO] Archive location: %ARCHIVE_DIR%

REM List built artifacts
for /r "%ARCHIVE_DIR%" %%F in (*.exe) do (
    echo [INFO] Built: %%F
)
goto :eof

REM ==============================================================================
REM CODE SIGNING
REM ==============================================================================
:cmd_sign
echo [STEP] Code signing Windows build...
echo.

if "%SKIP_SIGN%"=="true" (
    echo [WARN] Skipping code signing (--skip-sign)
    goto :eof
)

REM Validate signing environment
if not defined WIN_SIGNING_CERT (
    echo [ERROR] WIN_SIGNING_CERT is not set
    exit /b 1
)
if not exist "%WIN_SIGNING_CERT%" (
    echo [ERROR] Certificate file not found: %WIN_SIGNING_CERT%
    exit /b 1
)
if not defined WIN_SIGNING_PASSWORD (
    echo [ERROR] WIN_SIGNING_PASSWORD is not set
    exit /b 1
)

call :find_signtool
if errorlevel 1 exit /b 1

set "TIMESTAMP_URL=%WIN_TIMESTAMP_SERVER%"
if not defined TIMESTAMP_URL set "TIMESTAMP_URL=http://timestamp.digicert.com"

REM Find executables to sign
echo [INFO] Signing executables in: %ARCHIVE_DIR%
echo.

set "SIGN_COUNT=0"
for /r "%ARCHIVE_DIR%" %%F in (*.exe) do (
    echo [INFO] Signing: %%F
    "%WIN_SIGNTOOL_PATH%" sign ^
        /f "%WIN_SIGNING_CERT%" ^
        /p "%WIN_SIGNING_PASSWORD%" ^
        /tr "%TIMESTAMP_URL%" ^
        /td sha256 ^
        /fd sha256 ^
        /d "%PROJECT_DISPLAY_NAME%" ^
        /du "https://github.com/nio-racing-plus" ^
        "%%F"

    if errorlevel 1 (
        echo [ERROR] Failed to sign: %%F
        exit /b 1
    )
    set /a SIGN_COUNT+=1
)

REM Sign DLLs as well
for /r "%ARCHIVE_DIR%" %%F in (*.dll) do (
    echo [INFO] Signing DLL: %%F
    "%WIN_SIGNTOOL_PATH%" sign ^
        /f "%WIN_SIGNING_CERT%" ^
        /p "%WIN_SIGNING_PASSWORD%" ^
        /tr "%TIMESTAMP_URL%" ^
        /td sha256 ^
        /fd sha256 ^
        "%%F"

    if errorlevel 1 (
        echo [WARN] Failed to sign DLL (non-fatal): %%F
    ) else (
        set /a SIGN_COUNT+=1
    )
)

echo.
echo [INFO] Signed %SIGN_COUNT% files

REM Verify signatures
echo [STEP] Verifying signatures...
for /r "%ARCHIVE_DIR%" %%F in (*.exe) (
    "%WIN_SIGNTOOL_PATH%" verify /pa /v "%%F" 2>nul
    if errorlevel 1 (
        echo [WARN] Verification failed: %%F
    ) else (
        echo [INFO] Verified: %%F
    )
)

echo.
echo [INFO] Code signing completed successfully
goto :eof

REM ==============================================================================
REM PACKAGE
REM ==============================================================================
:cmd_package
echo [STEP] Creating Windows installer...
echo.

if "%SKIP_PACKAGE%"=="true" (
    echo [WARN] Skipping packaging (--skip-package)
    goto :eof
)

call :init_dirs

REM Find the main executable
set "MAIN_EXE="
for /r "%ARCHIVE_DIR%" %%F in ("%PROJECT_NAME%\Binaries\Win64\%PROJECT_NAME%.exe") do (
    set "MAIN_EXE=%%F"
)
if not defined MAIN_EXE (
    for /r "%ARCHIVE_DIR%" %%F in ("%PROJECT_NAME%.exe") do (
        set "MAIN_EXE=%%F"
    )
)

if /i "%INSTALLER_FRAMEWORK%"=="zip" goto :package_zip
if /i "%INSTALLER_FRAMEWORK%"=="nsis" goto :package_nsis
if /i "%INSTALLER_FRAMEWORK%"=="inno" goto :package_inno

echo [ERROR] Unknown installer framework: %INSTALLER_FRAMEWORK%
echo [ERROR] Use: nsis, inno, or zip
exit /b 1

REM --- ZIP Package ---
:package_zip
echo [INFO] Creating ZIP archive...

set "ZIP_NAME=%PROJECT_NAME%_%PROJECT_VERSION%_windows.zip"
set "ZIP_PATH=%PACKAGES_DIR%\%ZIP_NAME%"

REM Use PowerShell for ZIP creation
powershell -Command "Compress-Archive -Path '%ARCHIVE_DIR%\*' -DestinationPath '%ZIP_PATH%' -Force"

if errorlevel 1 (
    echo [ERROR] ZIP creation failed
    exit /b 1
)

REM Generate checksum
powershell -Command "(Get-FileHash '%ZIP_PATH%' -Algorithm SHA256).Hash" > "%ZIP_PATH%.sha256"
echo [INFO] SHA256: type "%ZIP_PATH%.sha256"

echo [INFO] ZIP created: %ZIP_PATH%
goto :package_done

REM --- NSIS Installer ---
:package_nsis
echo [INFO] Creating NSIS installer...

REM Check for NSIS
set "NSIS_PATH=%WIN_NSIS_PATH%"
if not defined NSIS_PATH set "NSIS_PATH=C:\Program Files (x86)\NSIS"
set "MAKENSIS=%NSIS_PATH%\makensis.exe"

if not exist "%MAKENSIS%" (
    echo [ERROR] NSIS not found at: %MAKENSIS%
    echo [ERROR] Install NSIS or set WIN_NSIS_PATH
    echo [INFO] Falling back to ZIP...
    set "INSTALLER_FRAMEWORK=zip"
    goto :package_zip
)

REM Generate NSIS script
set "NSIS_SCRIPT=%PACKAGES_DIR%\installer.nsi"
call :generate_nsis_script "%NSIS_SCRIPT%"

REM Build installer
"%MAKENSIS%" "%NSIS_SCRIPT%"

if errorlevel 1 (
    echo [ERROR] NSIS build failed
    exit /b 1
)

REM Sign the installer if signing is enabled
if "%SKIP_SIGN%"=="false" (
    if defined WIN_SIGNING_CERT (
        set "INSTALLER_FILE=%PACKAGES_DIR%\%PROJECT_NAME%_%PROJECT_VERSION%_Setup.exe"
        if exist "!INSTALLER_FILE!" (
            echo [INFO] Signing installer...
            "%WIN_SIGNTOOL_PATH%" sign ^
                /f "%WIN_SIGNING_CERT%" ^
                /p "%WIN_SIGNING_PASSWORD%" ^
                /tr "%TIMESTAMP_URL%" ^
                /td sha256 ^
                /fd sha256 ^
                /d "%PROJECT_DISPLAY_NAME% Setup" ^
                "!INSTALLER_FILE!"
        )
    )
)

REM Generate checksum
for %%F in ("%PACKAGES_DIR%\%PROJECT_NAME%*_Setup.exe") do (
    powershell -Command "(Get-FileHash '%%F' -Algorithm SHA256).Hash" > "%%F.sha256"
    echo [INFO] Installer: %%F
)

goto :package_done

REM --- Inno Setup ---
:package_inno
echo [INFO] Creating Inno Setup installer...

set "INNO_PATH=%WIN_INNO_PATH%"
if not defined INNO_PATH set "INNO_PATH=C:\Program Files (x86)\Inno Setup 6"
set "ISCC=%INNO_PATH%\ISCC.exe"

if not exist "%ISCC%" (
    echo [ERROR] Inno Setup not found at: %ISCC%
    echo [ERROR] Install Inno Setup 6 or set WIN_INNO_PATH
    echo [INFO] Falling back to ZIP...
    set "INSTALLER_FRAMEWORK=zip"
    goto :package_zip
)

REM Generate Inno Setup script
set "INNO_SCRIPT=%PACKAGES_DIR%\installer.iss"
call :generate_inno_script "%INNO_SCRIPT%"

REM Build installer
"%ISCC%" "%INNO_SCRIPT%"

if errorlevel 1 (
    echo [ERROR] Inno Setup build failed
    exit /b 1
)

REM Sign the installer
if "%SKIP_SIGN%"=="false" (
    if defined WIN_SIGNING_CERT (
        for %%F in ("%PACKAGES_DIR%\%PROJECT_NAME%*_Setup.exe") do (
            echo [INFO] Signing installer: %%F
            "%WIN_SIGNTOOL_PATH%" sign ^
                /f "%WIN_SIGNING_CERT%" ^
                /p "%WIN_SIGNING_PASSWORD%" ^
                /tr "%TIMESTAMP_URL%" ^
                /td sha256 ^
                /fd sha256 ^
                "%%F"
        )
    )
)

REM Generate checksum
for %%F in ("%PACKAGES_DIR%\%PROJECT_NAME%*_Setup.exe") do (
    powershell -Command "(Get-FileHash '%%F' -Algorithm SHA256).Hash" > "%%F.sha256"
    echo [INFO] Installer: %%F
)

goto :package_done

:package_done
echo.
echo [INFO] Packaging completed successfully
echo [INFO] Output: %PACKAGES_DIR%\
goto :eof

REM ==============================================================================
REM GENERATE NSIS SCRIPT
REM ==============================================================================
:generate_nsis_script
set "NSIS_OUT=%~1"
set "SOURCE_DIR=%ARCHIVE_DIR%"

(
echo ; NIO Racing Plus - NSIS Installer Script
echo ; Auto-generated by build_windows.bat
echo.
echo !define APP_NAME "%PROJECT_DISPLAY_NAME%"
echo !define APP_VERSION "%PROJECT_VERSION%"
echo !define APP_PUBLISHER "%PROJECT_COMPANY%"
echo !define APP_EXE "%PROJECT_NAME%.exe"
echo !define APP_DIR "%SOURCE_DIR%"
echo.
echo Name "${APP_NAME}"
echo OutFile "%PACKAGES_DIR%\%PROJECT_NAME%_%PROJECT_VERSION%_Setup.exe"
echo InstallDir "$PROGRAMFILES64\${APP_NAME}"
echo InstallDirRegKey HKLM "Software\${APP_NAME}" "InstallDir"
echo RequestExecutionLevel admin
echo Unicode True
echo.
echo ; Modern UI
echo !include "MUI2.nsh"
echo !define MUI_ABORTWARNING
echo.
echo ; Pages
echo !insertmacro MUI_PAGE_WELCOME
echo !insertmacro MUI_PAGE_LICENSE "..\..\LICENSE.txt"
echo !insertmacro MUI_PAGE_DIRECTORY
echo !insertmacro MUI_PAGE_INSTFILES
echo !insertmacro MUI_PAGE_FINISH
echo !insertmacro MUI_UNPAGE_CONFIRM
echo !insertmacro MUI_UNPAGE_INSTFILES
echo.
echo ; Language
echo !insertmacro MUI_LANGUAGE "English"
echo !insertmacro MUI_LANGUAGE "SimpChinese"
echo.
echo Section "Main Application" SecMain
echo     SectionIn RO
echo     SetOutPath "$INSTDIR"
echo.
echo     ; Install all files from archive
echo     File /r "${APP_DIR}\*.*"
echo.
echo     ; Create uninstaller
echo     WriteUninstaller "$INSTDIR\Uninstall.exe"
echo.
echo     ; Registry entries
echo     WriteRegStr HKLM "Software\${APP_NAME}" "InstallDir" "$INSTDIR"
echo     WriteRegStr HKLM "Software\${APP_NAME}" "Version" "${APP_VERSION}"
echo.
echo     ; Add/Remove Programs entry
echo     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
echo         "DisplayName" "${APP_NAME}"
echo     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
echo         "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
echo     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
echo         "DisplayVersion" "${APP_VERSION}"
echo     WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
echo         "Publisher" "${APP_PUBLISHER}"
echo     WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
echo         "NoModify" 1
echo     WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
echo         "NoRepair" 1
echo SectionEnd
echo.
echo Section "Desktop Shortcut" SecDesktop
echo     CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
echo SectionEnd
echo.
echo Section "Start Menu" SecStartMenu
echo     CreateDirectory "$SMPROGRAMS\${APP_NAME}"
echo     CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
echo     CreateShortcut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
echo SectionEnd
echo.
echo Section "Uninstall"
echo     ; Remove files
echo     RMDir /r "$INSTDIR"
echo.
echo     ; Remove shortcuts
echo     Delete "$DESKTOP\${APP_NAME}.lnk"
echo     RMDir /r "$SMPROGRAMS\${APP_NAME}"
echo.
echo     ; Remove registry entries
echo     DeleteRegKey HKLM "Software\${APP_NAME}"
echo     DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
echo SectionEnd
) > "%NSIS_OUT%"

echo [INFO] NSIS script generated: %NSIS_OUT%
goto :eof

REM ==============================================================================
REM GENERATE INNO SETUP SCRIPT
REM ==============================================================================
:generate_inno_script
set "INNO_OUT=%~1"
set "SOURCE_DIR=%ARCHIVE_DIR%"

(
echo [Setup]
echo AppName={#APP_NAME}
echo AppVersion={#APP_VERSION}
echo AppPublisher={#APP_PUBLISHER}
echo DefaultDirName={autopf}\{#APP_NAME}
echo DefaultGroupName={#APP_NAME}
echo OutputDir=%PACKAGES_DIR%
echo OutputBaseFilename=%PROJECT_NAME%_%PROJECT_VERSION%_Setup
echo Compression=lzma2/ultra64
echo SolidCompression=yes
echo ArchitecturesAllowed=x64compatible
echo ArchitecturesInstallIn64BitMode=x64compatible
echo SetupIconFile=
echo UninstallDisplayIcon={app}\{#APP_EXE}
echo.
echo [Languages]
echo Name: "english"; MessagesFile: "compiler:Default.isl"
echo Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"
echo.
echo [Tasks]
echo Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
echo.
echo [Files]
echo Source: "{#APP_DIR}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
echo.
echo [Icons]
echo Name: "{group}\{#APP_NAME}"; Filename: "{app}\{#APP_EXE}"
echo Name: "{group}\{cm:UninstallProgram,{#APP_NAME}}"; Filename: "{uninstallexe}"
echo Name: "{autodesktop}\{#APP_NAME}"; Filename: "{app}\{#APP_EXE}"; Tasks: desktopicon
echo.
echo [Run]
echo Filename: "{app}\{#APP_EXE}"; Description: "{cm:LaunchProgram,{#StringChange(APP_NAME, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
) > "%INNO_OUT%"

echo [INFO] Inno Setup script generated: %INNO_OUT%
goto :eof

REM ==============================================================================
REM VERIFY
REM ==============================================================================
:cmd_verify
echo [STEP] Verifying build signatures...
echo.

call :find_signtool
if errorlevel 1 exit /b 1

echo [INFO] Verifying executables in: %ARCHIVE_DIR%
echo.

for /r "%ARCHIVE_DIR%" %%F in (*.exe) (
    echo [INFO] Verifying: %%F
    "%WIN_SIGNTOOL_PATH%" verify /pa /v "%%F"
    if errorlevel 1 (
        echo [WARN] Verification failed: %%F
    ) else (
        echo [INFO] OK: %%F
    )
    echo.
)

REM Check installer
for %%F in ("%PACKAGES_DIR%\*Setup.exe") do (
    if exist "%%F" (
        echo [INFO] Verifying installer: %%F
        "%WIN_SIGNTOOL_PATH%" verify /pa /v "%%F"
        echo.
    )
)

echo [INFO] Verification complete
goto :eof

REM ==============================================================================
REM CLEAN
REM ==============================================================================
:cmd_clean
echo [STEP] Cleaning build artifacts...

if exist "%BUILD_DIR%" (
    echo [INFO] Removing: %BUILD_DIR%
    rmdir /s /q "%BUILD_DIR%"
)
if exist "Intermediate" (
    echo [INFO] Removing: Intermediate
    rmdir /s /q "Intermediate"
)
if exist "Binaries" (
    echo [INFO] Removing: Binaries
    rmdir /s /q "Binaries"
)
if exist "Saved\StagedBuilds" (
    echo [INFO] Removing: Saved\StagedBuilds
    rmdir /s /q "Saved\StagedBuilds"
)

echo [INFO] Clean completed
goto :eof

REM ==============================================================================
REM FULL RELEASE PIPELINE
REM ==============================================================================
:cmd_release
echo.
echo =============================================
echo   %PROJECT_DISPLAY_NAME% - Release Pipeline
echo =============================================
echo.

set "START_TIME=%TIME%"

REM Validate signing environment early
if "%SKIP_SIGN%"=="false" (
    if not defined WIN_SIGNING_CERT (
        echo [WARN] WIN_SIGNING_CERT not set. Use --skip-sign to skip signing.
        echo [WARN] Proceeding without signing...
        set "SKIP_SIGN=true"
    )
)

REM Build
call :cmd_build
if errorlevel 1 (
    echo [ERROR] Build stage failed
    exit /b 1
)

echo.
echo =============================================
echo   Build complete, starting sign stage...
echo =============================================
echo.

REM Sign
call :cmd_sign
if errorlevel 1 (
    echo [ERROR] Sign stage failed
    exit /b 1
)

echo.
echo =============================================
echo   Signing complete, starting packaging...
echo =============================================
echo.

REM Package
call :cmd_package
if errorlevel 1 (
    echo [ERROR] Package stage failed
    exit /b 1
)

echo.
echo =============================================
echo   Release Pipeline Complete
echo   Build Type: %BUILD_TYPE%
echo   Output:     %PACKAGES_DIR%\
echo =============================================
echo.

goto :eof

endlocal
