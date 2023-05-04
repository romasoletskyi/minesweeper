#include <iostream>

#include "mines.h"

using namespace game;
const static std::string mineState = "[[2 2 2 2 2 2 3 0 0]\n"
                                     " [3 3 2 3 3 4 4 0 0]\n"
                                     " [1 3 2 3 1 4 1 4 0]\n"
                                     " [3 3 2 3 3 0 0 0 0]\n"
                                     " [2 2 3 4 4 0 0 0 0]\n"
                                     " [2 2 3 1 1 0 0 0 0]\n"
                                     " [2 2 3 4 0 0 0 0 0]\n"
                                     " [2 2 3 3 0 0 0 0 0]\n"
                                     " [2 2 3 1 0 0 0 0 0]]";

void prepareState(State &state) {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            state[i][j] = std::stoi(mineState.substr((2 * WIDTH + 3) * i + 2 * j + 2, 1));
        }
    }
}

void printBoundary(const std::vector<std::pair<int, int>> &boundary) {
    for (const auto &[x, y]: boundary) {
        std::cout << x << " " << y << std::endl;
    }
}

bool getVariantValidity(const std::vector<Group> &constraints, const std::vector<bool> &variant) {
    for (const auto &group: constraints) {
        int sum = 0;
        for (int i: group.indices) {
            sum += variant[i];
        }
        if (sum != group.mines) {
            return false;
        }
    }
    return true;
}

int main() {
    State state;
    prepareState(state);

    auto boundary = getBoundary(state);
    printBoundary(boundary.getCoordinates());

    auto constraints = getMineConstraints(state, boundary);
    for (const auto &group: constraints) {
        for (int i: group.indices) {
            std::cout << i << " ";
        }
        std::cout << "| " << group.mines << std::endl;
    }

    auto variants = getMineVariants(state, constraints);
    for (const auto &variant: variants) {
        for (bool var: variant.variables) {
            std::cout << var;
        }
        std::cout << std::endl;
        if (!getVariantValidity(constraints, variant.variables)) {
            std::cout << "FALSE VARIANT" << std::endl;
        }
    }

    for (double p: getMineProbability(variants, getVariantProbability(state, variants))) {
        std::cout << p << " ";
    }
    return 0;
}