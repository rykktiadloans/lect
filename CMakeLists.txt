cmake_minimum_required(VERSION 3.30.0)

project(lect LANGUAGES CXX C VERSION 0.1.19)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(STRICT_COMPILE_COMMANDS -Wpedantic -Wall -Wextra -Wno-sign-conversion -Wno-unused-parameter -g)
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)

set(SRC_DIR src)
set(TREE_SITTER_LIB lib/tree-sitter-0.24.7/lib)
set(TREE_SITTER_CPP lib/tree-sitter-cpp)

## Comptime file reads
set(RESOURCES)
function(add_res_file STR_NAME FILE_NAME)
    file(READ res/${FILE_NAME} FILE_CONTENT)
    configure_file(res/filestringtemp.hpp ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/resread/${STR_NAME}.hpp)
    set(RESOURCES ${RESOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/resread/${STR_NAME}.hpp PARENT_SCOPE)
endfunction()

add_res_file(index_html index.html)
add_res_file(script_js script.js)
add_res_file(vis_js vis-network.min.js)

add_library(resources STATIC ${RESOURCES})
target_include_directories(resources PUBLIC ${SRC_DIR}/resread)
set_target_properties(resources PROPERTIES LINKER_LANGUAGE CXX)
target_compile_options(resources PRIVATE ${STRICT_COMPILE_COMMANDS})

## Dependencies
# Tree-sitter
add_library(tree-sitter STATIC
    ${TREE_SITTER_LIB}/src/lib.c
)

target_include_directories(tree-sitter 
    PRIVATE
    ${TREE_SITTER_LIB}/src
    ${TREE_SITTER_LIB}/include
    PUBLIC
    ${TREE_SITTER_LIB}/include
)

target_compile_options(tree-sitter PRIVATE ${STRICT_COMPILE_COMMANDS})

# C++ Grammar
add_library(tree-sitter-cpp STATIC
    ${TREE_SITTER_CPP}/src/parser.c
    ${TREE_SITTER_CPP}/src/scanner.c
)

target_include_directories(tree-sitter 
    PRIVATE
    ${TREE_SITTER_CPP}/src/tree_sitter
    ${TREE_SITTER_CPP}/bindings/c
    PUBLIC
    ${TREE_SITTER_CPP}/bindings/c
)

# JSON
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

target_compile_options(tree-sitter-cpp PRIVATE ${STRICT_COMPILE_COMMANDS})

## Library configuration
add_library(lect_lib STATIC
    ${SRC_DIR}/lect/extract.hpp
    ${SRC_DIR}/lect/export.hpp
    ${SRC_DIR}/lect/structures.hpp
    ${SRC_DIR}/lect/checks.hpp
    ${SRC_DIR}/lect/settings.hpp
    ${SRC_DIR}/lect/preprocessing.hpp
)

target_link_libraries(lect_lib PUBLIC tree-sitter tree-sitter-cpp nlohmann_json::nlohmann_json resources)
set_target_properties(lect_lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(lect_lib PUBLIC
    ${SRC_DIR}/lect
)

target_compile_options(lect_lib PRIVATE ${STRICT_COMPILE_COMMANDS})

## Target configuration
# Final executable
add_executable(lect ${SRC_DIR}/main.cpp)
target_link_libraries(lect lect_lib )
target_compile_options(lect PRIVATE ${STRICT_COMPILE_COMMANDS})
