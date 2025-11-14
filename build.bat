@echo off
setlocal

echo Building MarkdownStyler plugin...

:: Set up Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

:: Create build directory if it doesn't exist
if not exist "build" mkdir build

echo Current directory: %CD%

:: Compile the plugin with correct paths
echo Step 1: Compiling src\BetterMd.cpp...
cl.exe /nologo /O2 /MD /DUNICODE /D_UNICODE /DWIN32 /D_WINDOWS /D_USRDLL /DMARKDOWNSTYLER_EXPORTS /I"include" /I"plugin" /I"%NPP_SDK_PATH%\include" /c "src\BetterMd.cpp" /Fo"build\BetterMd.obj"

if %errorlevel% neq 0 (
    echo ERROR: Compilation failed!
    pause
    exit /b 1
)

:: Link the plugin
echo Step 2: Linking BetterMd.dll...
link.exe /nologo /DLL /OUT:"BetterMd.dll" /IMPLIB:"build\BetterMd.lib" "build\BetterMd.obj" user32.lib gdi32.lib kernel32.lib

if %errorlevel% equ 0 (
    echo.
    echo === BUILD SUCCESSFUL ===
    echo Plugin: BetterMd.dll
    echo.
    echo Copy to Notepad++ plugins directory
) else (
    echo.
    echo === BUILD FAILED ===
)

endlocal
pause
