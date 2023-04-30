cmake_minimum_required(VERSION 3.22)
project(minesweeper)

set(CMAKE_CXX_STANDARD 17)

set(PROJECT_SOURCE_DIR "src")
set(PROJECT_INCLUDE_DIR "include/minesweeper")

set(SOURCE_FILES ${PROJECT_INCLUDE_DIR}/mines.h ${PROJECT_SOURCE_DIR}/mines.cpp)

add_library(minesweeper SHARED ${SOURCE_FILES})
target_include_directories(minesweeper PUBLIC ${PROJECT_INCLUDE_DIR})

add_executable(test tests/test.cpp)
target_link_libraries(test PRIVATE minesweeper)
