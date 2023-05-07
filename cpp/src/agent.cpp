#include "agent.h"
#include "utils.h"

namespace agent {
    using namespace game;

    std::vector<game::Action> getExactActionsWeak(const game::State &state) {
        std::vector<Action> actions;
        for (const auto& [constraints, coordinates]: decoupleMineConstraints(getMineConstraints(state))) {
            int rows = static_cast<int>(constraints.size());
            int columns = static_cast<int>(coordinates.size());

            if (rows == 0 || columns == 0) {
                return {};
            }

            int matrix[rows][columns];
            int vector[rows];
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < columns; ++j) {
                    matrix[i][j] = 0;
                }
            }

            for (int i = 0; i < rows; ++i) {
                for (int index: constraints[i].indices) {
                    matrix[i][index] = 1;
                }
                vector[i] = constraints[i].mines;
            }

            makeGaussianElimination(&matrix[0][0], vector, rows, columns);
            std::unordered_map<int, bool> setVariables;

            for (int i = rows - 1; i >= 0; --i) {
                int lowerBound = 0;
                int upperBound = 0;
                std::vector<int> variables;

                for (int j = 0; j < columns; ++j) {
                    if (matrix[i][j]) {
                        variables.push_back(j);
                    }
                    lowerBound += std::min(matrix[i][j], 0);
                    upperBound += std::max(matrix[i][j], 0);
                }

                if (vector[i] == upperBound) {
                    for (int var: variables) {
                        setVariables[var] = matrix[i][var] > 0;
                    }
                }
                if (vector[i] == lowerBound) {
                    for (int var: variables) {
                        setVariables[var] = matrix[i][var] < 0;
                    }
                }
            }

            for (const auto &[index, mine]: setVariables) {
                auto [i, j] = coordinates[index];
                actions.emplace_back(Action{i, j, mine ? Cell::Flag : Cell::Open});
            }
        }

        return actions;
    }

    std::vector<game::Action> getExactActionsStrong(const game::State &state) {
        std::vector<Action> actions;
        for (const auto& [constraints, coordinates]: decoupleMineConstraints(getMineConstraints(state))) {
            auto variants = game::getMineVariants(constraints);

            std::unordered_map<int, bool> constantVariables;
            for (int i = 0; i < coordinates.size(); ++i) {
                constantVariables[i] = variants[0].variables[i];
            }

            std::vector<int> changedVariables;
            changedVariables.reserve(constantVariables.size());
            for (const auto &variant: variants) {
                for (const auto &[var, value]: constantVariables) {
                    if (variant.variables[var] != value) {
                        changedVariables.push_back(var);
                    }
                }
                for (int var: changedVariables) {
                    constantVariables.erase(var);
                }
                changedVariables.clear();
            }

            for (const auto &[var, value]: constantVariables) {
                auto [i, j] = coordinates[var];
                actions.emplace_back(Action{i, j, value ? Cell::Flag : Cell::Open});
            }
        }
        return actions;
    }

    std::vector<game::Action> RandomAgent::getActions(const State &state) {
        auto actions = getPossibleActions(state);
        if (actions.empty()) {
            return {};
        }

        auto action = actions[std::uniform_int_distribution<>(0, static_cast<int>(actions.size()) - 1)(gen_)];
        return {action};
    }

    game::GameResult RandomAgent::rollout(PerfectBoard board) {
        while (true) {
            auto actions = getActions(board.getState());
            for (auto action: actions) {
                auto result = board.act(action);
                if (isTerminal(result)) {
                    return result;
                }
            }
        }
    }

    std::vector<game::Action> TreeAgent::getActions(const State &state) {
        auto actions = getExactActionsWeak(state);
        if (!actions.empty()) {
            return actions;
        }

        tree_.moveto(state, getSimplePolicy(state));
        auto agent = RandomAgent(gen_);

        for (int i = 0; i < STEPS; ++i) {
            auto board = tree_.explore();
            if (!isTerminal(getStateResult(board.getState()))) {
                tree_.updateNode(getSimplePolicy(state), getReward(agent.rollout(board)));
            }
        }

        return {tree_.sampleAction()};
    }

    std::vector<double> TreeAgent::getSimplePolicy(const State &state) {
        size_t actionSpace = getPossibleActions(state).size();
        return std::vector<double>(actionSpace, 1.0 / (double) actionSpace);
    }
}