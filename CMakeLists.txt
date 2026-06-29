cmake_minimum_required(VERSION 3.16)
project(galganiza LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(galganiza_lib
    src/automatos/afd.cpp
    src/automatos/afnd.cpp
    src/automatos/algoritmos.cpp
    src/regex/compilador_regex.cpp
    src/regex/parser.cpp
    src/lexico/gerador_lexico.cpp
    src/sintatico/grammar.cpp
    src/sintatico/parser.cpp
    src/sintatico/tabelaSimbolos.cpp
)
target_include_directories(galganiza_lib PUBLIC include)
target_compile_options(galganiza_lib PRIVATE -Wall -Wextra -Wpedantic)

add_executable(galganiza_lexico src/main_lexico.cpp)
target_link_libraries(galganiza_lexico PRIVATE galganiza_lib)

add_executable(galganiza_sintatico src/main_sintatico.cpp)
target_link_libraries(galganiza_sintatico PRIVATE galganiza_lib)

enable_testing()
add_executable(galganiza_testes testes/testes.cpp)
target_link_libraries(galganiza_testes PRIVATE galganiza_lib)
add_test(NAME galganiza_testes COMMAND galganiza_testes)
