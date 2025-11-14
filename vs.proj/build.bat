@echo off
setlocal enabledelayedexpansion

echo =====================================
echo Building Better Markdown
echo =====================================
echo.

:: Locate vswhere
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% (
    echo ERROR: vswhere.exe not found. Install VS Build Tools 2022.
    pause
    exit /b 1
)

:: Find the latest installation with C++ tools
for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set VSROOT=%%i
)

if "%VSROOT%"=="" (
    echo ERROR: Visual Studio Build Tools not found!
    pause
    exit /b 1
)

echo Found Build Tools at: %VSROOT%
echo Initializing MSVC environment...

call "%VSROOT%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
if errorlevel 1 (
    echo ERROR: Failed to load compiler environment.
    pause
    exit /b 1
)

:: Ensure output folders exist
if not exist bin mkdir bin
if not exist obj mkdir obj

echo Compiling BetterMd.cpp...
cl ^
 /c ^
 /EHsc ^
 /Zi ^
 /DUNICODE ^
 /D_UNICODE ^
 /DNOMINMAX ^
 /DWIN32 ^
 /DWIN64 ^
 /I"..\plugin" ^
 /Foobj\BetterMd.obj ^
 "..\BetterMd.cpp"

if errorlevel 1 (
    echo Compilation failed.
    pause
    exit /b 1
)

echo Linking BetterMd.dll...

link ^
 /DLL ^
 /OUT:bin\BetterMd.dll ^
 obj\BetterMd.obj ^
 user32.lib gdi32.lib comctl32.lib

if errorlevel 1 (
    echo Linking failed!
    pause
    exit /b 1
)

echo.
echo ================================
echo BUILD SUCCESSFUL
echo ================================
echo Output: vs.proj\bin\BetterMd.dll
echo.
echo To install plugin:
echo   %%APPDATA%%\Notepad++\plugins\BetterMd\BetterMd.dll
echo.

pause
