cmake_minimum_required(VERSION 3.10)
project(handmade LANGUAGES CXX)

# ========== C++ Standard ==========
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========== Timestamp for PDB Naming ==========
string(TIMESTAMP BUILD_DATETIME "%Y%m%d_%H%M%S")
set(PDB_DLL_NAME "handmade_dll_${BUILD_DATETIME}")
set(PDB_EXE_NAME "handmade_exe_${BUILD_DATETIME}")
set(MAP_FILE_NAME "${CMAKE_SOURCE_DIR}/output/$<CONFIG>/bin/handmade.map")

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
    /wd4201     # Nameless struct/union
    /wd4100     # Unreferenced parameter
    /wd4189     # Unused variable
    /FC         # Full path in diagnostics
    /Zi         # Generate external PDB (required for linker output)
)

# ========== Linker Flags ==========
set(COMMON_LINKER_FLAGS
    /INCREMENTAL:NO
    /OPT:REF
    user32.lib
    gdi32.lib
    winmm.lib
    Kernel32.lib
)

# ========== DLL Target ==========
add_library(handmade SHARED
    code/handmade.cpp
    code/handmade.def
)

# DLL PDB & MAP file
set_target_properties(handmade PROPERTIES
    COMPILE_PDB_NAME ${PDB_DLL_NAME}
    COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/$<CONFIG>/bin"
    PDB_NAME ${PDB_DLL_NAME}
    PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/$<CONFIG>/bin"
)

target_link_options(handmade PRIVATE
    "/DEF:${CMAKE_SOURCE_DIR}/code/handmade.def"
    "/MAP"
    "/MAPINFO:EXPORTS"
    "/MAP:${MAP_FILE_NAME}"
)

# ========== EXE Target ==========
add_executable(handmade_exe
    code/win32_handmade.cpp
)

set_target_properties(handmade_exe PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    COMPILE_PDB_NAME handmade_exe
    COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/$<CONFIG>/bin"
    PDB_NAME handmade_exe
    PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/$<CONFIG>/bin"
)

target_link_libraries(handmade_exe PRIVATE handmade)

# ========== Preprocessor Macros & Flags ==========
foreach(target handmade handmade_exe)
    target_compile_definitions(${target} PRIVATE
        HANDMADE_WIN32=1
        $<$<CONFIG:Debug>:HANDMADE_INTERNAL=1>
        $<$<CONFIG:Debug>:HANDMADE_SLOW=1>
    )
    target_compile_options(${target} PRIVATE ${COMMON_COMPILER_FLAGS})
    target_link_options(${target} PRIVATE ${COMMON_LINKER_FLAGS})
endforeach()
