cmake_minimum_required(VERSION 3.10)
project(handmade LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Multi-config output directories
foreach(OUTPUTCONFIG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_SOURCE_DIR}/output/${OUTPUTCONFIG}/bin")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_SOURCE_DIR}/output/${OUTPUTCONFIG}/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_SOURCE_DIR}/output/${OUTPUTCONFIG}/lib")
endforeach()

add_executable(handmade WIN32
    code/win32_handmade.cpp
)

if(MSVC)
    target_compile_options(handmade PRIVATE
        /MT /nologo /Gm- /GR- /EHa- /Od /Oi /WX /W4 /wd4201 /wd4100 /wd4189 /FC /Z7
    )

    # Linker options with map file per config
    target_link_options(handmade PRIVATE
    "/SUBSYSTEM:WINDOWS,6.0"
    $<$<CONFIG:Release>:/OPT:REF /INCREMENTAL:NO /MAP:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/handmade.map>
    $<$<CONFIG:RelWithDebInfo>:/OPT:REF /INCREMENTAL:NO /MAP:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO}/handmade.map>
    $<$<CONFIG:MinSizeRel>:/OPT:REF /INCREMENTAL:NO /MAP:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL}/handmade.map>
    $<$<CONFIG:Debug>:/MAP:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/handmade.map>
)



    # Release debug info
    target_compile_options(handmade PRIVATE
        $<$<CONFIG:Release>:/Zi>
    )
    target_link_options(handmade PRIVATE
        $<$<CONFIG:Release>:/DEBUG>
    )
else()
    target_compile_options(handmade PRIVATE -Wall -Wextra -Werror)
    target_compile_options(handmade PRIVATE $<$<CONFIG:Release>:-g>)
endif()

target_compile_definitions(handmade PRIVATE
    HANDMADE_WIN32=1
    $<$<CONFIG:Debug>:HANDMADE_INTERNAL=1>
    $<$<CONFIG:Debug>:HANDMADE_SLOW=1>
)

target_link_libraries(handmade
    user32
    gdi32
    kernel32
    shell32
    ole32
)
