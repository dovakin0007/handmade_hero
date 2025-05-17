@echo off
setlocal

:: Remove build directory if it exists
if exist build (
    rmdir /s /q build
)

:: Recreate build directory
mkdir build
cd build

cmake --build . --target clean
:: Generate CMake files
cmake .. -DCMAKE_BUILD_TYPE=Debug || exit /b 1

:: Build the project in Debug mode

cmake --build . --config Debug || exit /b 1

endlocal
