#pragma once

#include "mines.h"
#include "simulation.h"
#include "tree.h"

namespace agent {
    std::vector<game::Action> getExactActions(const game::State &state);

    class RandomAgent {
    public:
        explicit RandomAgent(std::mt19937 &gen) : gen_(gen) {}

        std::vector<game::Action> getActions(const game::State &state);

        game::GameResult rollout(const game::State& state);

    private:
        std::mt19937 &gen_;
    };

    class TreeAgent {
    private:
        const int STEPS = 100;

    public:
        explicit TreeAgent(std::mt19937 &gen) : tree_(gen), gen_(gen) {}

        std::vector<game::Action> getActions(const game::State &state);

    private:
        static std::vector<double> getSimplePolicy(const game::State& state);

    private:
        tree::Tree tree_;
        std::mt19937 &gen_;
    };
}