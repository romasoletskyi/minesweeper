#include "agent.h"

namespace agent {
    using namespace game;

    std::vector<game::Action>
    getExactActions(const game::State &state, const std::vector<std::pair<int, int>> &coordinates,
                    const std::vector<double> &probability) {
        std::vector<Action> actions;
        for (int k = 0; k < coordinates.size(); ++k) {
            auto [i, j] = coordinates[k];
            if (probability[k] < 1e-6) {
                actions.emplace_back(Action{i, j, Cell::Open});
            }
            if (probability[k] > 1.0 - 1e-6) {
                actions.emplace_back(Action{i, j, Cell::Flag});
            }
        }

        if (probability.back() < 1e-6) {
            for (int i = 0; i < HEIGHT; ++i) {
                for (int j = 0; j < WIDTH; ++j) {
                    if (!isNeighbor(state, i, j) && !isFlagged(state, i, j)) {
                        actions.emplace_back(Action{i, j, Cell::Open});
                    }
                }
            }
        }

        return actions;
    }

    std::vector<game::Action> getExactActions(const game::State &state) {
        auto boundary = getBoundary(state);
        auto coordinates = boundary.getCoordinates();
        auto constraints = getMineConstraints(state, boundary);
        auto variants = getMineVariants(state, constraints);
        auto probability = getMineProbability(variants, getVariantProbability(state, variants));
        return getExactActions(state, coordinates, probability);
    }

    std::vector<game::Action> RandomAgent::getActions(const State &state) {
        auto boundary = getBoundary(state);
        auto coordinates = boundary.getCoordinates();
        auto constraints = getMineConstraints(state, boundary);
        auto variants = getMineVariants(state, constraints);
        auto probability = getMineProbability(variants, getVariantProbability(state, variants));

        auto actions = getExactActions(state, coordinates, probability);
        if (!actions.empty()) {
            return actions;
        }

        if (probability.size() == 1) {
            std::vector<std::pair<int, int>> indices;
            for (int i = 0; i < HEIGHT; ++i) {
                for (int j = 0; j < WIDTH; ++j) {
                    if (!isOpened(state, i, j) && !isFlagged(state, i, j)) {
                        indices.emplace_back(i, j);
                    }
                }
            }
            auto [i, j] = indices[std::uniform_int_distribution<>(0, static_cast<int>(indices.size()) - 1)(gen_)];
            return {Action{i, j, Cell::Open}};
        }

        size_t index = std::distance(probability.begin(), std::min_element(probability.begin(), --probability.end()));
        auto [i, j] = boundary.getCoordinates()[index];
        return {Action{i, j, Cell::Open}};
    }

    std::vector<game::Action> TreeAgent::getActions(const State &state) {
        tree_.moveto(state);
        auto board = PerfectBoard(gen_, state);
    }
}