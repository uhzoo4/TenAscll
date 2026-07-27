cmake_minimum_required(VERSION 3.10)
project(TENASCII C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(tenascii src/main.c src/tenascii.c)
add_executable(tenet src/tenet_sim.c)

if(UNIX)
    target_link_libraries(tenet m)
endif()

set_target_properties(tenascii tenet PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
