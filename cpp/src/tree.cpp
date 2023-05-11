#include <valarray>
#include "tree.h"

namespace tree {
    void Tree::moveto(game::State state, std::vector<double> policy) {
        if (states_.count(state)) {
            root_ = states_.at(state);
        } else {
            root_ = nullptr;
        }

        std::unordered_set<Node *> states;
        std::vector<Node *> nodes;
        if (!root_) {
            root_ = createNode(state, nullptr, 0);
            root_->policy = std::move(policy);
        }
        rootAnalysis_ = game::analyzeState(state);
        root_->parent = nullptr;
        nodes.push_back(root_);

        while (!nodes.empty()) {
            auto node = nodes.back();
            nodes.pop_back();
            states.insert(node);

            for (auto child: node->children) {
                nodes.push_back(child);
            }
        }

        for (const auto &[_, node]: states_) {
            if (!states.count(node)) {
                delete node;
            }
        }

        states_.clear();
        for (auto node: states) {
            states_[node->state] = node;
        }
    }

    game::PerfectBoard Tree::explore() {
        int it = 0;
        while (true) {
            Node *parent = nullptr;
            Node *node = root_;

            int actionIndexBest;
            auto board = game::PerfectBoard(gen_, rootAnalysis_);

            while (node && !(node->terminal)) {
                int nVisitsSum = 0;
                double uValues[node->actions.size()];

                for (int n: node->nVisits) {
                    nVisitsSum += n;
                }

                double uBest;
                for (int i = 0; i < node->actions.size(); ++i) {
                    uValues[i] =
                            node->qValues[i] + CPUCT * node->policy[i] * std::sqrt(nVisitsSum) / (node->nVisits[i] + 1);
                    if (i == 0 || uBest < uValues[i]) {
                        uBest = uValues[i];
                        actionIndexBest = i;
                    }
                }

                board.act(node->actions[actionIndexBest]);

                parent = node;
                if (states_.count(board.getState())) {
                    node = states_.at(board.getState());
                    node->parent = parent;
                    node->parentActionIndex = actionIndexBest;
                } else {
                    node = nullptr;
                }
            }

            if (!node) {
                node = createNode(board.getState(), parent, actionIndexBest);
                states_[board.getState()] = node;
                parent->children.insert(node);
                updated_ = node;
            }

            if (node->terminal) {
                propagateValue(node, node->value);
            } else {
                return board;
            }
            if (++it == MAXITER) {
                return board;
            }
        }
    }

    void Tree::updateNode(std::vector<double> policy, double value) {
        updated_->policy = std::move(policy);
        updated_->value = value;
        propagateValue(updated_, value);
    }

    Tree::Node *Tree::createNode(const game::State &state, Node *parent, int parentActionIndex) {
        auto node = new Node();
        node->parent = parent;
        node->parentActionIndex = parentActionIndex;

        node->state = state;
        node->actions = game::getPossibleActions(state);
        node->nVisits = std::vector<int>(node->actions.size());
        node->qValues = std::vector<double>(node->actions.size());

        std::uniform_real_distribution<> unif(-1e-8, 1e-8);
        for (double &qValue: node->qValues) {
            qValue = unif(gen_);
        }

        auto result = game::getStateResult(state);
        node->terminal = game::isTerminal(result);
        if (node->terminal) {
            node->value = game::getReward(result);
        }

        return node;
    }

    void Tree::propagateValue(Tree::Node *node, double value) {
        int action = node->parentActionIndex;
        node = node->parent;

        while (node) {
            node->qValues[action] =
                    (node->qValues[action] * node->nVisits[action] + value) / (node->nVisits[action] + 1);
            ++(node->nVisits[action]);

            action = node->parentActionIndex;
            node = node->parent;
        }
    }

    game::Action Tree::sampleAction() const {
        std::vector<int> cumulative(root_->nVisits.size());
        std::partial_sum(root_->nVisits.begin(), root_->nVisits.end(), cumulative.begin());

        size_t index = std::distance(cumulative.begin(),
                                     std::lower_bound(cumulative.begin(), cumulative.end(),
                                                      std::uniform_int_distribution<>(1, cumulative.back())(gen_)));
        return root_->actions[index];
    }
}
