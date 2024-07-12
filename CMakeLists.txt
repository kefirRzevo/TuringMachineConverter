cmake_minimum_required (VERSION 3.16)
project (converter)

set(CMAKE_CXX_FLAGS "-O0 -g")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

find_package(Boost 1.40 COMPONENTS program_options REQUIRED)

#main
set (SOURCES main.cc)

add_executable (${PROJECT_NAME} ${SOURCES})
target_include_directories (${PROJECT_NAME} PRIVATE includes)
target_include_directories (${PROJECT_NAME} PRIVATE include ${Boost_INCLUDE_DIR})

add_compile_options (-Werror -Wall -Wextra -Wpedantic)
target_link_libraries(${PROJECT_NAME} ${Boost_PROGRAM_OPTIONS_LIBRARY})
