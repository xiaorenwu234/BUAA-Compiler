cmake_minimum_required(VERSION 3.27)
project(Compiler)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(SOURCES
        src/function.cpp
        src/category.cpp
        src/main.cpp
        src/lexical_analysis.cpp
        src/grammer_analysis.cpp
        src/getfirst.cpp
        src/symbletable.cpp
        src/intermediate_code.cpp
        src/calculate_exp.cpp
        src/target_code.cpp
)

add_executable(Compiler ${SOURCES})
