@echo off
setlocal

:: Default to Debug if no argument is provided
set "BUILD_TYPE=%1"
if "%BUILD_TYPE%"=="" (
    set "BUILD_TYPE=Debug"
)

echo Building configuration: %BUILD_TYPE%

:: Remove build directory if it exists
if exist build (
    rmdir /s /q build
)

:: Recreate build directory
mkdir build
cd build

:: Clean previous build artifacts
cmake --build . --target clean >nul 2>&1

:: Set HANDMADE_SLOW and HANDMADE_INTERNAL for Debug builds
set "EXTRA_DEFINES="
if /I "%BUILD_TYPE%"=="Debug" (
    set "EXTRA_DEFINES=-DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1"
)

:: Generate CMake files with selected build type and extra defines
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %EXTRA_DEFINES% || exit /b 1

:: Build the project
cmake --build . --config %BUILD_TYPE% || exit /b 1

endlocal
