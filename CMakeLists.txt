cmake_minimum_required(VERSION 3.22)
project(minesweeper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined -fno-sanitize-recover=all"
        CACHE STRING "Compiler flags in asan build"
        FORCE)

set(PROJECT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cpp/include/minesweeper")
set(SOURCE_FILES
        cpp/src/agent.cpp
        cpp/src/mines.cpp
        cpp/src/simulation.cpp
        cpp/src/tree.cpp)
set(HEADER_FILES
        cpp/include/minesweeper/agent.h
        cpp/include/minesweeper/mines.h
        cpp/include/minesweeper/simulation.h
        cpp/include/minesweeper/state.h
        cpp/include/minesweeper/tree.h
        cpp/include/minesweeper/utils.h)
set(PYTHON_FILES
        python/minesweeper.h
        python/minesweeper.cpp)

include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/python")

add_library(minesweeper SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(minesweeper PUBLIC ${PROJECT_INCLUDE_DIR})

include(pybind11.cmake)
pybind11_add_module(engine
        ${SOURCE_FILES}
        ${HEADER_FILES}
        ${PYTHON_FILES}
        )

target_link_libraries(engine PUBLIC)

install(TARGETS engine
        COMPONENT python
        LIBRARY DESTINATION "${PYTHON_LIBRARY_DIR}"
        )

add_subdirectory(cpp/tests)

add_executable(main tests/test.cpp ${PYTHON_FILES})
target_link_libraries(main PRIVATE minesweeper pybind11::embed)
