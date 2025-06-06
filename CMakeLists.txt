cmake_minimum_required(VERSION 3.10)
project(handmade LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output")

# Add the executable for a Win32 GUI app
add_executable(handmade WIN32
    code/win32_handmade.cpp
)

# Link against required Win32 libraries
target_link_libraries(handmade
    user32
    gdi32
    kernel32
    shell32
    Ole32
)
