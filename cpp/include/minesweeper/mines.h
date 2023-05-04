#pragma once

#include <cstddef>
#include <utility>
#include <vector>
#include <unordered_map>

#include "state.h"

namespace game {
    class Boundary {
    public:
        std::vector<std::pair<int, int>> getCoordinates() const {
            std::vector<std::pair<int, int>> co;
            for (int c: coordinates_) {
                co.emplace_back(unpackPair(c));
            }
            return co;
        }

        int getIndex(int i, int j) const {
            return indices_.at(packPair(i, j));
        }

    private:
        static std::pair<int, int> unpackPair(int c) {
            return {c >> 16, c & 0xffff};
        }

        static int packPair(int i, int j) {
            return i << 16 | j;
        }

    private:
        friend class BoundaryBuilder;

        std::vector<int> coordinates_;
        std::unordered_map<int, int> indices_;
    };

    class BoundaryBuilder {
    public:
        void addPoint(int i, int j) {
            coordinates_.push_back(Boundary::packPair(i, j));
        }

        Boundary build() {
            Boundary boundary;
            boundary.coordinates_ = std::move(coordinates_);
            for (int i = 0; i < boundary.coordinates_.size(); ++i) {
                boundary.indices_[boundary.coordinates_[i]] = i;
            }
            return boundary;
        }

    private:
        std::vector<int> coordinates_;
    };

    Boundary getBoundary(const State &state);

    struct Group {
        std::vector<int> indices;
        int mines;
    };

    std::vector<Group> getMineConstraints(const State &state, const Boundary &boundary);

    struct Variant {
        std::vector<bool> variables;
        int leftSpace;
        int leftMines;
    };

    std::vector<Variant> getMineVariants(const State &state, const std::vector<Group> &constraints);

    std::vector<double> getVariantProbability(const State &state, const std::vector<Variant> &variants);

    std::vector<double>
    getMineProbability(const std::vector<Variant> &variants, const std::vector<double> &variantProbability);
}