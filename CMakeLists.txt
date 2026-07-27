cmake_minimum_required(VERSION 3.10)
project(tenascii C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall")

add_executable(tenet
    src/main.c
    src/term.c
    src/particle.c
    src/algorithm.c
    src/render.c
)

target_include_directories(tenet PRIVATE include)

# math.h needs -lm on Linux/macOS; MinGW on Windows doesn't need (or always
# has) a separate libm, so only link it on non-Windows targets.
if (NOT WIN32)
    target_link_libraries(tenet PRIVATE m)
endif()