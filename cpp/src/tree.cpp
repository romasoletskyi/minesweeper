#include <valarray>
#include "tree.h"

namespace tree {
    void Tree::moveto(const game::State &state) {
        if (states_.count(state)) {
            root_ = states_.at(state);
        } else {
            root_ = nullptr;
        }

        std::unordered_set<Node*> states;
        std::vector<Node*> nodes;
        if (!root_) {
            nodes.push_back(root_);
        }

        while (!nodes.empty()) {
            auto node = nodes.back();
            nodes.pop_back();
            states.insert(node);

            for (auto child: node->children) {
                nodes.push_back(child);
            }
        }

        for (const auto& [_, node]: states_) {
            if (!states.count(node)) {
                delete node;
            }
        }

        states_.clear();
        for (auto node: states) {
            states_[node->state] = node;
        }
    }

    game::State Tree::explore(const game::PerfectBoard& board) {
        while (true) {
            Node *parent = nullptr;
            Node *node = root_;

            int actionIndexBest;
            game::State state;

            auto boardCopy = board;

            while (!node && !(node->terminal)) {
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

                boardCopy.act(node->actions[actionIndexBest]);
                state = boardCopy.getState();

                parent = node;
                if (states_.count(state)) {
                    node = states_.at(state);
                    node->parent = parent;
                    node->parentActionIndex = actionIndexBest;
                } else {
                    node = nullptr;
                }
            }

            if (!node) {
                node = createNode(state, parent, actionIndexBest);
                states_[state] = node;
                if (!parent) {
                    root_ = node;
                } else {
                    parent->children.insert(node);
                }
                updated_ = node;
            }

            if (node->terminal) {
                propagateValue(node, node->value);
            } else {
                return state;
            }
        }
    }

    void Tree::updateNode(const std::vector<double> &policy, double value) {
        updated_->policy = policy;
        updated_->value = value;
        propagateValue(updated_, value);
    }

    Tree::Node *Tree::createNode(const game::State &state, Node* parent, int parentActionIndex) {
        auto node = new Node();
        node->parent = parent;
        node->parentActionIndex = parentActionIndex;

        node->state = state;
        node->actions = game::getPossibleActions(state);
        node->nVisits = std::vector<int>(node->actions.size());
        node->qValues = std::vector<double>(node->actions.size());

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

        while (!node) {
            node->qValues[action] =
                    (node->qValues[action] * node->nVisits[action] + value) / (node->nVisits[action] + 1);
            ++(node->nVisits[action]);

            action = node->parentActionIndex;
            node = node->parent;
        }
    }
}
