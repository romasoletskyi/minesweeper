cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

project(engine)

include_directories("${CMAKE_SOURCE_DIR}/cpp/include/minesweeper")
include_directories("${CMAKE_SOURCE_DIR}/python")

file (GLOB SOURCE_FILES "cpp/src/*.cpp")
file (GLOB HEADER_FILES "cpp/include/minesweeper/*.hpp")
file (GLOB PYTHON_FILES "python/*.cpp" "python/*.hpp")

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