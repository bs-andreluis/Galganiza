cmake_minimum_required(VERSION 3.16)
project(gal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(gal_lib
    src/automaton.cpp
    src/regex.cpp
    src/lexer_generator.cpp
)
target_include_directories(gal_lib PUBLIC include)
target_compile_options(gal_lib PRIVATE -Wall -Wextra -Wpedantic)

add_executable(gal src/main.cpp)
target_link_libraries(gal PRIVATE gal_lib)

enable_testing()
add_executable(gal_tests tests/tests.cpp)
target_link_libraries(gal_tests PRIVATE gal_lib)
add_test(NAME gal_tests COMMAND gal_tests)

