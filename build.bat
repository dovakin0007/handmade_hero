@echo off
setlocal enabledelayedexpansion

:: Default to Debug if no argument is provided
set "BUILD_TYPE=%1"
if "%BUILD_TYPE%"=="" (
    set "BUILD_TYPE=Debug"
)

echo ==================================================
echo Building configuration: %BUILD_TYPE%
echo ==================================================

:: Remove build directory if it exists
if exist build (
    echo Removing existing build directory...
    rmdir /s /q build
)

:: Recreate build directory
mkdir build
cd build

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
