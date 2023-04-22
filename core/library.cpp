#include "library.h"

bool isOpened(const State &state, int i, int j) {
    return state[i][j] >= 2;
}

int getMineCount(const State &state, int i, int j) {
    return state[i][j] - 2;
}

bool isNeighbor(const State &state, int i, int j) {
    for (int n = std::max(i - 1, 0); n < std::min(i + 2, HEIGHT); ++n) {
        for (int m = std::max(j - 1, 0); m < std::min(j + 2, WIDTH); ++m) {
            if (isOpened(state, n, m)) {
                return true;
            }
        }
    }
    return false;
}

Boundary getBoundary(const State &state) {
    BoundaryBuilder builder;
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            if (isNeighbor(state, i, j) && !isOpened(state, i, j)) {
                builder.addPoint(i, j);
            }
        }
    }
    return builder.build();
}

std::vector<Group> getMineConstraints(const State &state, const Boundary &boundary) {
    std::vector<Group> groups;

    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            if (isOpened(state, i, j)) {
                std::vector<int> indices;

                for (int n = std::max(i - 1, 0); n < std::min(i + 2, HEIGHT); ++n) {
                    for (int m = std::max(j - 1, 0); m < std::min(j + 2, WIDTH); ++m) {
                        if (!isOpened(state, n, m)) {
                            indices.push_back(boundary.getIndex(n, m));
                        }
                    }
                }

                if (!indices.empty()) {
                    groups.emplace_back(Group{indices, getMineCount(state, i, j)});
                }
            }
        }
    }

    std::sort(groups.begin(), groups.end(), [](const Group &lhs, const Group &rhs) {
        return lhs.indices.size() > rhs.indices.size();
    });

    return groups;
}

std::vector<Group> reduceConstraints(const std::vector<Group> &constraints,
                                     const std::unordered_map<int, bool> &setVariables) {
    std::vector<Group> groups;
    for (const auto &group: constraints) {
        std::vector<int> indices;
        int mines = group.mines;

        for (int i: group.indices) {
            if (setVariables.count(i)) {
                mines -= setVariables.at(i);
            } else {
                indices.push_back(i);
            }
        }

        if (!indices.empty()) {
            groups.push_back(Group{indices, mines});
        }
    }

    std::sort(groups.begin(), groups.end(), [](const Group &lhs, const Group &rhs) {
        return lhs.indices.size() > rhs.indices.size();
    });

    return groups;
}

class BranchIterator {
public:
    explicit BranchIterator(size_t size) : branch_(size) {

    }

    const std::vector<bool> &operator*() const {
        return branch_;
    }

    BranchIterator &operator++() {
        int i = 0;
        while (i < branch_.size() && branch_[i]) {
            branch_[i] = false;
            ++i;
        }
        if (i == branch_.size()) {
            overflow_ = true;
        } else {
            branch_[i] = true;
        }
        return *this;
    }

    bool overflow() const {
        return overflow_;
    }

private:
    std::vector<bool> branch_;
    bool overflow_ = false;
};

void setMines(std::vector<Group> constraints, std::unordered_map<int, bool> setVariables,
              std::vector<std::vector<bool>> &variants) {
    while (!constraints.empty() && constraints.back().indices.size() == 1) {
        while (!constraints.empty() && constraints.back().indices.size() == 1) {
            setVariables[constraints.back().indices.back()] = constraints.back().mines;
            constraints.pop_back();
        }
        constraints = reduceConstraints(constraints, setVariables);
    }
    if (constraints.empty()) {
        std::vector<bool> variant(setVariables.size());
        for (const auto &[index, variable]: setVariables) {
            variant[index] = variable;
        }
        variants.push_back(variant);
    } else {
        auto constraint = std::move(constraints.back());
        constraints.pop_back();

        BranchIterator it(constraint.indices.size());
        do {
            int sum = 0;
            for (int var: *it) {
                sum += var;
            }
            if (sum == constraint.mines) {
                auto branchSetVariables = setVariables;
                for (int i = 0; i < constraint.indices.size(); ++i) {
                    branchSetVariables[constraint.indices[i]] = (*it)[i];
                }
                setMines(reduceConstraints(constraints, setVariables), branchSetVariables, variants);
            }
        } while (!(++it).overflow());
    }
}

std::vector<std::vector<bool>> getMineVariants(const std::vector<Group> &constraints) {
    std::vector<std::vector<bool>> variants;
    std::unordered_map<int, bool> setVariables;
    setMines(constraints, setVariables, variants);
    return variants;
}
