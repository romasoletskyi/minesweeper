add_executable(solver test_solver.cpp)
add_executable(simulation test_simulation.cpp)
add_executable(utils test_utils.cpp)

target_link_libraries(solver PRIVATE minesweeper)
target_link_libraries(simulation PRIVATE minesweeper)
target_link_libraries(utils PRIVATE minesweeper)
