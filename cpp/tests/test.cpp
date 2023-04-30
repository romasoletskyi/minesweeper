#include <iostream>

#include "mines.h"

void prepareState(State& state) {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            state[i][j] = 0;
        }
    }

    state[0][0] = state[1][0] = state[2][0] = state[2][1] = 2;
    state[0][1] = state[1][1] = state[2][2] = state[3][0] = state[3][1] = 3;
    state[3][2] = 4;
    state[1][2] = 5;
}

void printBoundary(const std::vector<std::pair<int, int>>& boundary) {
    for (const auto& [x, y]: boundary) {
        std::cout << x << " " << y << std::endl;
    }
}

bool getVariantValidity(const std::vector<Group>& constraints, const std::vector<bool>& variant) {
    for (const auto& group: constraints) {
        int sum = 0;
        for (int i : group.indices) {
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
    for (const auto& group: constraints) {
        for (int i: group.indices) {
            std::cout << i << " ";
        }
        std::cout << "| " << group.mines << std::endl;
    }

    auto variants = getMineVariants(constraints);
    for (const auto& variant: variants) {
        for (bool var: variant) {
            std::cout << var;
        }
        std::cout << std::endl;
        if (!getVariantValidity(constraints, variant)) {
            std::cout << "FALSE VARIANT" << std::endl;
        }
    }

    for (double p: getMineProbability(state, variants)) {
        std::cout << p << " ";
    }
    return 0;
}