# Note that this project requires MSVC and a single-config generator (Makefiles or Ninja) to build.

cmake_minimum_required(VERSION 3.21)

if(NOT CMAKE_BUILD_TYPE)
    message(NOTICE "Setting CMAKE_BUILD_TYPE to Release")
    set(CMAKE_BUILD_TYPE Release)
endif()

project(tccswizzle LANGUAGES C)

set(CL_OPTS_1 [[/DUNICODE /D_UNICODE /Gs- /W3 /wd"5105" /Gy- /Zc:wchar_t /Gm-]])
set(CL_OPTS_2 [[/sdl- /Zc:inline /fp:precise /std:c11 /Gd /Oi]])
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CL_OPTS_1} ${CL_OPTS_2}")
string(REPLACE "/O2" "/Ox" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_C_STANDARD_LIBRARIES [["kernel32.lib" "user32.lib" "gdi32.lib" "advapi32.lib"]])

add_library(swizzle MODULE
    "src/swizzle.c"
    "src/functions.c"
)

target_link_libraries(swizzle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/TakeCmd.lib")
target_link_options(swizzle PUBLIC "/DYNAMICBASE" "/NXCOMPAT")
set_target_properties(swizzle PROPERTIES
    VERSION 0.1
    OUTPUT_NAME swizzle
)

target_precompile_headers(swizzle PRIVATE "src/pch.h")
