@echo off
setlocal

:: Default to Debug if no argument is provided
set "BUILD_TYPE=%1"
if "%BUILD_TYPE%"=="" (
    set "BUILD_TYPE=Debug"
)

echo ==================================================
echo Building configuration: %BUILD_TYPE%
echo ==================================================

:: Remove build directory if it exists (full cleanup)
if exist build (
    echo Removing existing build directory...
    rmdir /s /q build
)

:: Recreate build directory
mkdir build
cd build

:: Set HANDMADE_SLOW and HANDMADE_INTERNAL for Debug builds
set "EXTRA_DEFINES="
if /I "%BUILD_TYPE%"=="Debug" (
    set "EXTRA_DEFINES=-DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1"
)

:: Configure with CMake
echo Configuring CMake...
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %EXTRA_DEFINES%
if errorlevel 1 exit /b 1

:: Build the project
echo Building project...
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 exit /b 1

echo ==================================================
echo Build completed successfully!
echo ==================================================

endlocal
