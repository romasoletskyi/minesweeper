#include <iostream>

#include "mines.h"
#include "agent.h"

using namespace game;
const static std::string mineState = "[[2 3 4 0 4 1 3 2 2]\n"
                                     " [2 3 1 1 4 3 3 2 2]\n"
                                     " [3 4 5 4 3 2 2 3 3]\n"
                                     " [0 0 3 2 2 2 2 3 1]\n"
                                     " [3 0 3 2 2 3 3 4 3]\n"
                                     " [3 0 3 2 2 4 1 0 0]\n"
                                     " [3 0 4 3 4 5 1 4 0]\n"
                                     " [0 3 0 0 0 0 0 0 0]\n"
                                     " [0 0 3 3 4 3 0 0 0]]";

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

std::vector<game::Action> getExactActionsSlow(const game::State &state) {
    auto [constraints, coordinates] = game::getMineConstraints(state);
    auto variants = game::getMineVariants(constraints);

    std::unordered_map<int, bool> constantVariables;
    for (int i = 0; i < coordinates.size(); ++i) {
        constantVariables[i] = variants[0].variables[i];
    }

    std::vector<int> changedVariables;
    for (const auto& variant: variants) {
        for (const auto& [var, value]: constantVariables) {
            if (variant.variables[var] != value) {
                changedVariables.push_back(var);
            }
        }
        for (int var: changedVariables) {
            constantVariables.erase(var);
        }
        changedVariables.clear();
    }

    std::vector<Action> actions;
    for (const auto& [var, value]: constantVariables) {
        auto [i, j] = coordinates[var];
        actions.emplace_back(Action{i, j, value ? Cell::Flag : Cell::Open});
    }

    return actions;
}

int main() {
    State state;
    prepareState(state);

    auto constraint = getMineConstraints(state);
    printBoundary(constraint.coordinates);

    for (const auto &group: constraint.groups) {
        for (int i: group.indices) {
            std::cout << i << " ";
        }
        std::cout << "| " << group.mines << std::endl;
    }

    auto variants = getMineVariants(constraint.groups);
    for (const auto &variant: variants) {
        for (bool var: variant.variables) {
            std::cout << var;
        }
        std::cout << std::endl;
        if (!getVariantValidity(constraint.groups, variant.variables)) {
            std::cout << "FALSE VARIANT" << std::endl;
        }
    }

    auto actionSlow = getExactActionsSlow(state);
    auto action = agent::getExactActions(state);

    std::cout << "slow" << std::endl;
    for (auto x: actionSlow) {
        std::cout << x.i << " " << x.j << " " << (x.cell ? Cell::Flag : Cell::Open) << std::endl;
    }
    std::cout << "fast" << std::endl;
    for (auto x: action) {
        std::cout << x.i << " " << x.j << " " << (x.cell ? Cell::Flag : Cell::Open) << std::endl;
    }

    auto analysis = analyzeState(state);
    return 0;
}