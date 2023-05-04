cmake_minimum_required(VERSION 3.22)
project(minesweeper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined -fno-sanitize-recover=all"
        CACHE STRING "Compiler flags in asan build"
        FORCE)

set(PROJECT_INCLUDE_DIR "include/minesweeper")
set(SOURCE_FILES
        src/agent.cpp
        src/mines.cpp
        src/simulation.cpp
        src/state.cpp
        src/tree.cpp)
set(HEADER_FILES
        include/minesweeper/agent.h
        include/minesweeper/mines.h
        include/minesweeper/simulation.h
        include/minesweeper/state.h
        include/minesweeper/tree.h)

add_library(minesweeper SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(minesweeper PUBLIC ${PROJECT_INCLUDE_DIR})

add_subdirectory(tests)
