#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>

const static int HEIGHT = 16;
const static int WIDTH = 30;

// 0 - unknown, 1 - mine, n + 2 - open (n mines surrounding)
using State = uint8_t[HEIGHT][WIDTH];

class Boundary {
public:
    auto getCoordinates() const {
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

std::vector<std::vector<bool>> getMineVariants(const std::vector<Group> &constraints);