#pragma once

#include "mines.h"

enum Cell {
    Free,
    Mine
};

struct Action {
    int i, j;
    Cell cell;
};

class ExactAgent {
public:
    void loadState(const State& state) {
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                state_[i][j] = state[i][j];
            }
        }
    }

    std::vector<Action> getActions() const {
        std::vector<Action> actions;
        auto boundary = getBoundary(state_);
        auto coordinates = boundary.getCoordinates();
        auto constraints = getMineConstraints(state_, boundary);
        auto variants = getMineVariants(constraints);
        auto probability = getMineProbability(state_, variants);

        for (int k = 0; k < coordinates.size(); ++k) {
            auto [i, j] = coordinates[k];
            if (probability[k] == 0.0) {
                actions.emplace_back(Action{i, j, Cell::Free});
            }
            if (probability[k] == 1.0) {
                actions.emplace_back(Action{i, j, Cell::Mine});
            }
        }

        if (probability.back() == 0.0) {
            for (int i = 0; i < HEIGHT; ++i) {
                for (int j = 0; j < WIDTH; ++j) {
                    if (!isNeighbor(state_, i, j) && !isFlagged(state_, i, j)) {
                        actions.emplace_back(Action{i, j, Cell::Free});
                    }
                }
            }
        }

        return actions;
    }

private:
    State state_;
};
