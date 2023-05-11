#pragma once

#include <optional>
#include "mines.h"
#include "simulation.h"
#include "tree.h"

namespace agent {
    std::vector<game::Action> getExactActionsWeak(const game::State &state);

    std::vector<game::Action> getExactActionsStrong(const game::State &state);

    class RandomAgent {
    public:
        explicit RandomAgent(std::mt19937 &gen) : gen_(gen) {}

        std::vector<game::Action> getActions(const game::State &state);

        game::GameResult rollout(game::PerfectBoard board);

    private:
        std::mt19937 &gen_;
    };

    class TreeAgent {
    public:
        explicit TreeAgent(std::mt19937 &gen) : tree_(gen), gen_(gen) {}

        TreeAgent(const TreeAgent &agent) : tree_(agent.tree_), gen_(agent.gen_) {}

        TreeAgent &operator=(const TreeAgent &agent) {
            *this = TreeAgent(agent);
            return *this;
        }

        TreeAgent(TreeAgent &&agent) : tree_(std::move(agent.tree_)), gen_(agent.gen_) {}

        TreeAgent &operator=(TreeAgent &&agent) {
            tree_ = std::move(agent.tree_);
            gen_ = agent.gen_;
            return *this;
        }

        void loadState(game::State state, std::vector<double> policy);

        std::vector<game::Action> getActions(const game::State &state);

        std::optional<game::State> explore();

        void update(std::vector<double> policy, double value);

        std::optional<int> getIter() const {
            return iter_;
        }

        const game::State &getState() const {
            return tree_.getRoot()->state;
        }

    private:
        tree::Tree tree_;
        std::mt19937 &gen_;
        std::optional<int> iter_;
    };

    class SimpleTreeAgent {
    private:
        const int STEPS = 50;

    public:
        explicit SimpleTreeAgent(std::mt19937 &gen) : tree_(gen), gen_(gen) {}

        std::vector<game::Action> getActions(const game::State &state);

    private:
        static std::vector<double> getSimplePolicy(const game::State &state);

    private:
        tree::Tree tree_;
        std::mt19937 &gen_;
    };
}