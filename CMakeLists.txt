cmake_minimum_required(VERSION 3.17.0)

project(lect CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(STRICT_COMPILE_COMMANDS -Wpedantic -Wall -Wextra -Wno-sign-conversion -Wno-unused-parameter -g)

add_compile_options(${STRICT_COMPILE_COMMANDS})

set(SRC_DIR src)

# Library configuration
add_library(lect_lib STATIC
    ${SRC_DIR}/lect/extract.hpp
)

set_target_properties(lect_lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(lect_lib PUBLIC
    ${SRC_DIR}/lect
)


# Target configuration
add_executable(lect ${SRC_DIR}/main.cpp)
target_link_libraries(lect lect_lib)
