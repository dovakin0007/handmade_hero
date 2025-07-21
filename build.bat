@echo off
setlocal enabledelayedexpansion

:: Default to Debug if no argument is provided
set "BUILD_TYPE=%1"
if "%BUILD_TYPE%"=="" (
    set "BUILD_TYPE=Debug"
)

:: Check if "clean" was passed as a second argument
set "CLEAN_BUILD=false"
if /I "%2"=="clean" (
    set "CLEAN_BUILD=true"
)

echo ==================================================
echo Building configuration: %BUILD_TYPE%
echo Clean build: %CLEAN_BUILD%
echo ==================================================

:: Remove build directory only if clean is specified
if /I "%CLEAN_BUILD%"=="true" (
    if exist build (
        echo Removing existing build directory...
        rmdir /s /q build
    )
)

:: Recreate build directory if it doesn't exist
if not exist build (
    mkdir build
)
cd build

for %%X in (pdb map) do (
    for /r "%~dp0output" %%f in (*.%%X) do (
        del "%%f" >nul 2>&1
    )
)

:: Prepare extra CMake flags for Debug builds
set "EXTRA_CMAKE_FLAGS="
if /I "%BUILD_TYPE%"=="Debug" (
    set "EXTRA_CMAKE_FLAGS=-DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1"
)

:: Run CMake configuration
echo Configuring CMake...
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% !EXTRA_CMAKE_FLAGS!
if errorlevel 1 (
    echo CMake configuration failed.
    exit /b 1
)

:: Build the project
echo Building project...
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo ==================================================
echo Build completed successfully!
echo ==================================================

endlocal
