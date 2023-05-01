cmake_minimum_required(VERSION 3.22)
project(minesweeper)

set(CMAKE_CXX_STANDARD 17)

set(PROJECT_INCLUDE_DIR "include/minesweeper")
set(SOURCE_FILES
        src/agent.cpp
        src/mines.cpp
        src/simulation.cpp)
set(HEADER_FILES
        include/minesweeper/agent.h
        include/minesweeper/mines.h
        include/minesweeper/simulation.h)

add_library(minesweeper SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(minesweeper PUBLIC ${PROJECT_INCLUDE_DIR})

add_subdirectory(tests)
