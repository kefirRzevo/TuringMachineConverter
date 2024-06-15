cmake_minimum_required (VERSION 3.16)
project (converter)

set(CMAKE_CXX_FLAGS "-O0 -g")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

#main
set (SOURCES main.cpp)

add_executable (${PROJECT_NAME} ${SOURCES})
target_include_directories (${PROJECT_NAME} PRIVATE includes)

add_compile_options (-Werror -Wall -Wextra -Wpedantic)

#tests
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
