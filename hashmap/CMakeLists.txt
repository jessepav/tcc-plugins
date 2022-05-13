# Note that this project requires MSVC and a single-config generator (Makefiles or Ninja) to build.

cmake_minimum_required(VERSION 3.21)

option(TCCHM_DEBUG "Compile hashmap plugin with debug messages" OFF)
option(USE_PCH "Use precompiled headers" ON)

if(NOT CMAKE_BUILD_TYPE)
    message(NOTICE "Setting CMAKE_BUILD_TYPE to Release")
    set(CMAKE_BUILD_TYPE Release)
endif()

project(TCCHashmap LANGUAGES C)

set(CL_OPTS_1 [[/DUNICODE /D_UNICODE /Gs- /W3 /WX /wd"5105" /Gy- /Zc:wchar_t /Gm-]])
set(CL_OPTS_2 [[/sdl- /Zc:inline /fp:precise /std:c11 /Gd /Oi]])
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CL_OPTS_1} ${CL_OPTS_2}")
string(REPLACE "/O2" "/Ox" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_C_STANDARD_LIBRARIES [["kernel32.lib" "user32.lib" "gdi32.lib" "advapi32.lib" "Shell32.lib"]])

add_library(hashmap MODULE
    "src/tcchashmap.c"
    "src/hashmap.c"
    "src/hashmap.rc"
)

target_link_libraries(hashmap PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/TakeCmd.lib")
target_link_options(hashmap PUBLIC "/DYNAMICBASE" "/NXCOMPAT")
set_target_properties(hashmap PROPERTIES
    VERSION 1.1
    OUTPUT_NAME hashmap
)

if(TCCHM_DEBUG)
    message(NOTICE "Compiling with debug messages")
    target_compile_definitions(hashmap PRIVATE TCCHM_DEBUG=1)
endif()

if(USE_PCH)
    message(NOTICE "Using precompiled headers")
    target_precompile_headers(hashmap PRIVATE "src/pch.h")
endif()
