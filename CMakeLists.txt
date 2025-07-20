cmake_minimum_required(VERSION 3.10)
project(handmade LANGUAGES CXX)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========== Output Directories ==========
foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/output/${CONFIG}/bin")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${OUTPUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${OUTPUT_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${OUTPUT_DIR})
endforeach()

# ========== Compiler Flags ==========
set(COMMON_COMPILER_FLAGS
    /MT         # Statically link CRT
    /nologo     # Suppress startup banner
    /Gm-        # Disable minimal rebuild
    /GR-        # Disable RTTI
    /EHa-       # Disable exception handling
    /Od         # Disable optimizations
    /Oi         # Enable intrinsics
    /WX         # Warnings as errors
    /W4         # Warning level 4
    /wd4201     # Disable warning: nameless struct/union
    /wd4100     # Disable warning: unreferenced formal parameter
    /wd4189     # Disable warning: initialized but unused variable
    /FC         # Full path in diagnostics
    /Z7         # Debug info in .obj files
)

# ========== Linker Flags ==========
set(COMMON_LINKER_FLAGS
    /INCREMENTAL:NO
    /OPT:REF
    user32.lib
    gdi32.lib
    winmm.lib
)

# ========== DLL Target ==========
add_library(handmade SHARED
    code/handmade.cpp
    code/handmade.def  # <-- contains export list
)

# ========== EXE Target ==========
add_executable(handmade_exe
    code/win32_handmade.cpp
)

# Set the subsystem to WINDOWS to use WinMain instead of main
set_target_properties(handmade_exe PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)

# Link executable to the DLL import library
target_link_libraries(handmade_exe PRIVATE handmade)

# ========== Preprocessor & Flags ==========
foreach(target handmade handmade_exe)
    # Define preprocessor macros
    target_compile_definitions(${target} PRIVATE
        HANDMADE_WIN32=1
        $<$<CONFIG:Debug>:HANDMADE_INTERNAL=1>
        $<$<CONFIG:Debug>:HANDMADE_SLOW=1>
    )

    # Apply MSVC flags
    target_compile_options(${target} PRIVATE ${COMMON_COMPILER_FLAGS})
    target_link_options(${target} PRIVATE ${COMMON_LINKER_FLAGS})
endforeach()

# Optional: Set subsystem to Windows XP-compatible (optional)
# set_target_properties(handmade_exe PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS,5.1")
