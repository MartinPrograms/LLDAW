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

if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif ()

# Add the most efficient optimization flags, and the -ffast-math flag
if (MSVC)
    add_compile_options(/O2 /fp:fast)
else ()
    add_compile_options(-O3 -ffast-math)
endif ()

# Apple Silicon NEON
if (APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    add_compile_options(-march=armv8-a+simd)
endif()

# AVX2 detection (works on Windows/Linux/macOS)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    if (MSVC)
        add_compile_options(/arch:AVX2)
    else()
        add_compile_options(-mavx2 -mfma -mavx)
    endif()
endif()

# Add memory leak detection flags
if (MSVC)
    add_compile_options(/fsanitize=address)
    add_link_options(/fsanitize=address)
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-fsanitize=address -static-libsan)
    add_link_options(-fsanitize=address)
endif ()

# Link the files in ./libs/tinycthread
add_library(tinycthread STATIC ./libs/tinycthread/tinycthread.c)
target_include_directories(tinycthread PUBLIC ./libs/tinycthread)

add_library(clay INTERFACE)
target_include_directories(clay INTERFACE ./libs/clay)

add_library(kiss_fft STATIC ./libs/kiss_fft/kiss_fft.c)
target_include_directories(kiss_fft PUBLIC ./libs/kiss_fft)

add_subdirectory(libs/portmidi EXCLUDE_FROM_ALL)

# Override the include directories to use relative paths
get_target_property(PORTMIDI_INCLUDES portmidi INTERFACE_INCLUDE_DIRECTORIES)

# For building port midi, disable all warnings as errors (it has a lot of warnings, at least on MinGW)
if (MSVC)
    set_property(TARGET portmidi APPEND_STRING PROPERTY COMPILE_FLAGS "/W0")
else ()
    set_property(TARGET portmidi APPEND_STRING PROPERTY COMPILE_FLAGS "-w")
endif ()

# link all files from the src/* directory and their subdirectories
file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(LLDAW ${SOURCES})

target_link_libraries(LLDAW raylib)
target_link_libraries(LLDAW tinycthread)
target_link_libraries(LLDAW clay)
target_link_libraries(LLDAW kiss_fft)
target_link_libraries(LLDAW portmidi)

if (APPLE)
    target_link_libraries(LLDAW "-framework IOKit")
    target_link_libraries(LLDAW "-framework Cocoa")
    target_link_libraries(LLDAW "-framework OpenGL")
endif()

# Copy the ./resources directory to the build directory
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})
