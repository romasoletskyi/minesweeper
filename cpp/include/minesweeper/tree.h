#pragma once

#include <unordered_set>

#include "mines.h"
#include "simulation.h"

namespace tree {
    class Tree {
    private:
        const double CPUCT = 1;

        struct Node {
            game::State state;

            std::vector<game::Action> actions;
            std::vector<int> nVisits;
            std::vector<double> qValues;

            std::vector<double> policy;
            double value;
            bool terminal;

            Node *parent = nullptr;
            int parentActionIndex;
            std::unordered_set<Node *> children;
        };

    public:
        void moveto(const game::State& state);

        game::State explore(const game::PerfectBoard& board);

        void updateNode(const std::vector<double> &policy, double value);

        ~Tree() {
            for (const auto& [_, node]: states_) {
                delete node;
            }
        }

    private:
        Node* createNode(const game::State& state, Node* parent, int parentActionIndex);

        void propagateValue(Node *node, double value);

    private:
        std::unordered_map<game::State, Node*> states_;
        Node *root_ = nullptr;
        Node *updated_ = nullptr;
    };
}
