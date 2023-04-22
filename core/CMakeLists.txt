cmake_minimum_required(VERSION 3.23)
project(core)

set(CMAKE_CXX_STANDARD 17)

add_library(core library.cpp)

add_executable(main main.cpp)
target_link_libraries(main core)
