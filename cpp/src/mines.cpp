#include <cmath>
#include <algorithm>
#include <numeric>
#include <optional>

#include "mines.h"

bool isOpened(const State &state, int i, int j) {
    return state[i][j] >= EMPTY;
}

bool isFlagged(const State &state, int i, int j) {
    return state[i][j] == FLAG;
}

int getMineCount(const State &state, int i, int j) {
    return state[i][j] - EMPTY;
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
            if (isNeighbor(state, i, j) && !isOpened(state, i, j) && !isFlagged(state, i, j)) {
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
                int mines = getMineCount(state, i, j);
                std::vector<int> indices;

                for (int n = std::max(i - 1, 0); n < std::min(i + 2, HEIGHT); ++n) {
                    for (int m = std::max(j - 1, 0); m < std::min(j + 2, WIDTH); ++m) {
                        if (!isOpened(state, n, m)) {
                            if (isFlagged(state, n, m)) {
                                --mines;
                            } else {
                                indices.push_back(boundary.getIndex(n, m));
                            }
                        }
                    }
                }

                if (!indices.empty()) {
                    groups.emplace_back(Group{indices, mines});
                }
            }
        }
    }

    std::sort(groups.begin(), groups.end(), [](const Group &lhs, const Group &rhs) {
        return lhs.indices.size() > rhs.indices.size();
    });

    return groups;
}

std::optional<std::vector<Group>> reduceConstraints(const std::vector<Group> &constraints,
                                                    const std::unordered_map<int, bool> &setVariables) {
    std::vector<Group> groups;
    for (const auto &group: constraints) {
        std::vector<int> indices;
        int mines = group.mines;

        for (int i: group.indices) {
            if (setVariables.count(i)) {
                mines -= setVariables.at(i);
                if (mines < 0) {
                    return std::nullopt;
                }
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
            if (constraints.back().mines > 1) {
                return;
            }
            setVariables[constraints.back().indices.back()] = constraints.back().mines;
            constraints.pop_back();
        }

        auto reduced = reduceConstraints(constraints, setVariables);
        if (reduced.has_value()) {
            constraints = std::move(*reduced);
        } else {
            return;
        }
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

                auto reduced = reduceConstraints(constraints, branchSetVariables);
                if (reduced.has_value()) {
                    setMines(reduced.value(), branchSetVariables, variants);
                }
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

double diffLogFactorial(int lhs, int rhs) {
    if (lhs < rhs) {
        return -diffLogFactorial(rhs, lhs);
    }
    double diff = 0;
    for (int i = rhs + 1; i <= lhs; ++i) {
        diff += std::log(i);
    }
    return diff;
}

std::vector<double> getMineProbability(const State &state, const std::vector<std::vector<bool>> &variants) {
    int flags = 0;
    int open = 0;

    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            if (isFlagged(state, i, j)) {
                ++flags;
            }
            if (isOpened(state, i, j)) {
                ++open;
            }
        }
    }

    std::vector<std::pair<int, int>> binomial;
    std::vector<std::vector<bool>> variantsFiltered;
    for (const auto &variant: variants) {
        int sum = 0;
        for (int var: variant) {
            sum += var;
        }
        if (flags + sum <= MINES) {
            binomial.emplace_back(HEIGHT * WIDTH - flags - open - variant.size(), MINES - flags - sum);
            variantsFiltered.push_back(variant);
        }
    }

    auto [bestSize, bestMines] = *std::max_element(binomial.begin(), binomial.end(),
                                                   [](const auto &lhs, const auto &rhs) {
                                                       double lhsRate = (double) lhs.second / lhs.first;
                                                       double rhsRate = (double) rhs.second / rhs.first;
                                                       return std::abs(lhsRate - 0.5) > std::abs(rhsRate - 0.5);
                                                   });

    std::vector<double> relative;
    for (const auto &[size, mines]: binomial) {
        double log = diffLogFactorial(size, bestSize) - diffLogFactorial(mines, bestMines) -
                     diffLogFactorial(size - mines, bestSize - bestMines);
        relative.push_back(std::exp(log));
    }

    double sum = std::accumulate(relative.begin(), relative.end(), 0.0);
    std::vector<double> probability;
    for (double x: relative) {
        probability.push_back(x / sum);
    }

    std::vector<double> mineProbability(variantsFiltered.back().size());
    for (int i = 0; i < variantsFiltered.size(); ++i) {
        for (int j = 0; j < variantsFiltered[i].size(); ++j) {
            mineProbability[j] += variantsFiltered[i][j] * probability[i];
        }
    }

    double externalProbability = 0;
    for (int i = 0; i < variantsFiltered.size(); ++i) {
        auto [size, mines] = binomial[i];
        externalProbability += probability[i] * (double) mines / size;
    }

    mineProbability.push_back(externalProbability);
    return mineProbability;
}
