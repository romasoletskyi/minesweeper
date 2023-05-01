add_executable(solver test_solver.cpp)
add_executable(simulation test_simulation.cpp)

target_link_libraries(solver PRIVATE minesweeper)
target_link_libraries(simulation PRIVATE minesweeper)
