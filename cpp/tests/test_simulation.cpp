#include <iostream>

#include "simulation.h"
#include "agent.h"

using namespace game;

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
    std::mt19937 gen;
    Board board(gen);

    while (true) {
        char command;
        int i, j;
        std::cin >> command >> i >> j;

        if (command == 'o') {
            auto result = board.act(Action{i, j, Cell::Open});
            if (result != GameResult::Continue) {
                if (result == GameResult::Win) {
                    std::cout << "WIN" << std::endl;
                } else {
                    std::cout << "LOSE" << std::endl;
                }
                break;
            }
        } else if (command == 'f') {
            board.act(Action{i, j, Cell::Flag});
        }

        printState(board.getState());
    }
}

bool simple(std::mt19937& gen, bool verbose) {
    Board board(gen);
    agent::RandomAgent agent(gen);

    while(true) {
        auto actions = agent.getActions(board.getState());
        for (auto action: actions) {
            auto result = board.act(action);
            if (result != GameResult::Continue) {
                if (result == GameResult::Win) {
                    return true;
                } else {
                    return false;
                }
            }
            if (verbose) {
                printState(board.getState());
                printState(board.getSecretState());
            }
        }
    }
}

int main() {
    // interactive();

    int games = 100;
    int won = 0;
    std::mt19937 gen(42);

    for (int i = 0; i < games; ++i) {
        bool result = simple(gen, false);
        won += result;
    }
    std::cout << "won " << won << "/" << games;
    return 0;
}