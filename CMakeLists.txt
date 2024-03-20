cmake_minimum_required(VERSION 3.27.4)

project(mukar_scheme)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE_EXE main.cpp)

add_library(scheme
    src/tokenizer.cpp
    src/parser.cpp
    src/scheme.cpp
    src/object.cpp
)

add_executable(main ${SOURCE_EXE})

target_link_libraries(main scheme)
