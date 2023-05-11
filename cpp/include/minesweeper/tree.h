#pragma once

#include <unordered_set>

#include "mines.h"
#include "simulation.h"

namespace tree {
    class Tree {
    private:
        const double CPUCT = 1;
        const int MAXITER = 10;

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
        explicit Tree(std::mt19937 &gen) : gen_(gen) {}

        Tree(const Tree &tree) : rootAnalysis_(tree.rootAnalysis_), gen_(tree.gen_) {
            root_ = copyNode(tree.root_, nullptr, states_);
            updated_ = states_[tree.updated_->state];
        }

        Tree &operator=(const Tree &tree) {
            return *this = Tree(tree);
        }

        Tree(Tree &&tree) : states_(std::move(tree.states_)), rootAnalysis_(std::move(tree.rootAnalysis_)),
                            gen_(tree.gen_), root_(tree.root_), updated_(tree.updated_) {
        }

        Tree &operator=(Tree &&tree) {
            std::swap(states_, tree.states_);
            std::swap(rootAnalysis_, tree.rootAnalysis_);
            gen_ = tree.gen_;
            root_ = tree.root_;
            updated_ = tree.updated_;
            return *this;
        }

        ~Tree() {
            for (const auto &[_, node]: states_) {
                delete node;
            }
        }

        void moveto(game::State state, std::vector<double> policy);

        game::PerfectBoard explore();

        void updateNode(std::vector<double> policy, double value);

        game::Action sampleAction() const;

        Node *getUpdatedNode() const {
            return updated_;
        }

        Node *getRoot() const {
            return root_;
        }

    private:
        Node *createNode(const game::State &state, Node *parent, int parentActionIndex);

        Node *copyNode(Node* node, Node* parent, std::unordered_map<game::State, Node*>& states) {
            auto copy = new Node(*node);
            states[copy->state] = copy;
            copy->children.clear();
            copy->parent = parent;

            for (auto child: node->children) {
                copy->children.insert(copyNode(child, copy, states));
            }

            return copy;
        }

        void propagateValue(Node *node, double value);

    private:
        std::unordered_map<game::State, Node *> states_;
        game::StateAnalysis rootAnalysis_;
        std::mt19937 &gen_;
        Node *root_ = nullptr;
        Node *updated_ = nullptr;
    };
}
