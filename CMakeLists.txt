cmake_minimum_required(VERSION 3.10)
project(handmade LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Handle multi-config output directories (e.g., for Visual Studio)
foreach(OUTPUTCONFIG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_SOURCE_DIR}/output/${OUTPUTCONFIG}")
endforeach()

# Add the executable for a Win32 GUI app
add_executable(handmade WIN32
    code/win32_handmade.cpp
)

# Define debug-only macros
target_compile_definitions(handmade PRIVATE
    $<$<CONFIG:Debug>:HANDMADE_SLOW=1>
    $<$<CONFIG:Debug>:HANDMADE_INTERNAL=1>
)

# Link against required Win32 libraries
target_link_libraries(handmade
    user32
    gdi32
    kernel32
    shell32
    Ole32
)

# Add debug info to Release builds
if(MSVC)
    # /Zi: Debug info, /DEBUG: linker flag for PDB
    target_compile_options(handmade PRIVATE
        $<$<CONFIG:Release>:/Zi>
    )
    target_link_options(handmade PRIVATE
        $<$<CONFIG:Release>:/DEBUG>
    )
else()
    # GCC/Clang: -g for debug info
    target_compile_options(handmade PRIVATE
        $<$<CONFIG:Release>:-g>
    )
endif()
