cmake_minimum_required(VERSION 3.20)
project(LLDAW C)

set(CMAKE_C_STANDARD 23)

set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    endif()
endif()

# Enable warnings as errors in the msvc compiler
if (MSVC)
    add_compile_options(/W4 /WX)
else ()
    add_compile_options(-Wall -Wextra -Werror)
endif ()

# Link the files in ./libs/tinycthread
add_library(tinycthread STATIC ./libs/tinycthread/tinycthread.c)
target_include_directories(tinycthread PUBLIC ./libs/tinycthread)

# link all files from the src/* directory and their subdirectories
file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(LLDAW ${SOURCES})

target_link_libraries(LLDAW raylib)
target_link_libraries(LLDAW tinycthread)

if (APPLE)
    target_link_libraries(LLDAW "-framework IOKit")
    target_link_libraries(LLDAW "-framework Cocoa")
    target_link_libraries(LLDAW "-framework OpenGL")
endif()

