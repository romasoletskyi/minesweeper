#include <iostream>
#include "simulation.h"

void printState(const State& state) {
    std::cout << "┌";
    for (int i = 0; i < WIDTH; ++i) {
        std::cout << "─";
    }
    std::cout << "┐\n";

    for (int i = 0; i < HEIGHT; ++i) {
        std::cout << "│";
        for (int j = 0; j < WIDTH; ++j) {
            if (isOpened(state, i, j)) {
                int mines = getMineCount(state, i, j);
                if (mines > 0) {
                    std::cout << mines;
                } else {
                    std::cout << "·";
                }
            } else if (isFlagged(state, i, j)) {
                std::cout << "*";
            } else {
                std::cout << " ";
            }
        }
        std::cout << "│\n";
    }

    std::cout << "└";
    for (int i = 0; i < WIDTH; ++i) {
        std::cout << "─";
    }
    std::cout << "┘\n" << std::endl;
}

void interactive() {
    Board board;

    while (true) {
        char command;
        int i, j;
        std::cin >> command >> i >> j;

        if (command == 'o') {
            auto result = board.open(i, j);
            if (result != GameResult::Continue) {
                if (result == GameResult::Win) {
                    std::cout << "WIN" << std::endl;
                } else {
                    std::cout << "LOSE" << std::endl;
                }
                break;
            }
        } else if (command == 'f') {
            board.flag(i, j);
        }

        printState(board.getState());
    }
}

int main() {
    interactive();
    return 0;
}