cmake_minimum_required(VERSION 3.10)
project(TENASCII C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB SOURCES 
    "src/main.c"
    "src/term.c"
    "src/particle.c"
    "src/algorithm.c"
    "src/render.c"
)

add_executable(tenet ${SOURCES})

if(UNIX)
    target_link_libraries(tenet m)
endif()

set_target_properties(tenet PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
