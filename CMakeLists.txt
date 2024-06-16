cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

add_compile_options (-Werror -Wall -Wextra -Wpedantic)

project(machines)

find_package(Boost 1.40 COMPONENTS program_options REQUIRED)

add_executable (ETM lib/ExecuteTM.cc)
target_include_directories (ETM PRIVATE includes)
target_include_directories (ETM PRIVATE include ${Boost_INCLUDE_DIR})
target_link_libraries(ETM ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable (CTM lib/ConvertTM.cc)
target_include_directories (CTM PRIVATE includes)
target_include_directories (CTM PRIVATE include ${Boost_INCLUDE_DIR})
target_link_libraries(CTM ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable (ETS lib/ExecuteTS.cc)
target_include_directories (ETS PRIVATE includes)
target_include_directories (ETS PRIVATE include ${Boost_INCLUDE_DIR})
target_link_libraries(ETS ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable (CTS lib/ConvertTS.cc)
target_include_directories (CTS PRIVATE includes)
target_include_directories (CTS PRIVATE include ${Boost_INCLUDE_DIR})
target_link_libraries(CTS ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable (ECTS lib/ExecuteCTS.cc)
target_include_directories (ECTS PRIVATE includes)
target_include_directories (ECTS PRIVATE include ${Boost_INCLUDE_DIR})
target_link_libraries(ECTS ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable (${PROJECT_NAME} main.cc)
target_include_directories (${PROJECT_NAME} PRIVATE includes)
target_include_directories (${PROJECT_NAME} PRIVATE include ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${Boost_PROGRAM_OPTIONS_LIBRARY})
