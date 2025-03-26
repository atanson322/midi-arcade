@echo off
echo ===== MIDI Arcade Rebuild Script =====
echo Cleaning previous build...

REM Clean previous build directories
if exist "Builds\VisualStudio2022\x64\Debug" (
    rmdir /s /q "Builds\VisualStudio2022\x64\Debug"
)

echo Generating project files with Projucer...

REM Find Projucer and generate project files
for /f "delims=" %%i in ('where /r "c:\juce-proj\JUCE" Projucer.exe') do (
    echo Found Projucer at: %%i
    "%%i" --resave MidiArcade.jucer
)

echo Building project with MSBuild...

REM Find MSBuild
set "MSBUILD_PATH="
for /f "delims=" %%i in ('where /r "C:\Program Files\Microsoft Visual Studio" MSBuild.exe') do (
    set "MSBUILD_PATH=%%i"
    goto :found_msbuild
)
:found_msbuild

if "%MSBUILD_PATH%"=="" (
    echo MSBuild not found. Please make sure Visual Studio is installed.
    exit /b 1
)

echo Using MSBuild: %MSBUILD_PATH%

REM Build the project
"%MSBUILD_PATH%" Builds\VisualStudio2022\MidiArcade.sln /p:Configuration=Debug /p:Platform=x64

echo Build completed.
echo.
echo Standalone application: Builds\VisualStudio2022\x64\Debug\Standalone Plugin\MidiArcade.exe
echo VST3 plugin: Builds\VisualStudio2022\x64\Debug\VST3\MidiArcade.vst3
echo.
