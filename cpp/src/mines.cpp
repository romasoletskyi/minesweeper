#include <cmath>
#include <algorithm>
#include <numeric>
#include <optional>

#include "mines.h"

namespace game {
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

    std::vector<Variant> getMineVariants(const State &state, const std::vector<Group> &constraints) {
        std::vector<std::vector<bool>> variants;
        std::unordered_map<int, bool> setVariables;
        setMines(constraints, setVariables, variants);

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

        std::vector<Variant> variantsFiltered;

        for (const auto &variant: variants) {
            int sum = 0;
            for (int var: variant) {
                sum += var;
            }
            if (flags + sum <= MINES) {
                variantsFiltered.emplace_back(Variant{variant,
                                                      HEIGHT * WIDTH - flags - open - (int) variant.size(),
                                                      MINES - flags - sum});
            }
        }

        return variantsFiltered;
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

    std::vector<double> getVariantProbability(const State &state, const std::vector<Variant> &variants) {
        auto bestVariant = *std::max_element(variants.begin(), variants.end(), [](const auto &lhs, const auto &rhs) {
            double lhsRate = (double) lhs.leftMines / lhs.leftSpace;
            double rhsRate = (double) rhs.leftMines / rhs.leftSpace;
            return std::abs(lhsRate - 0.5) > std::abs(rhsRate - 0.5);
        });

        int bestSpace = bestVariant.leftSpace;
        int bestMines = bestVariant.leftMines;
        std::vector<double> relative;
        relative.reserve(variants.size());

        for (const auto& variant: variants) {
            int space = variant.leftSpace;
            int mines = variant.leftMines;
            double log = diffLogFactorial(space, bestSpace) - diffLogFactorial(mines, bestMines) -
                         diffLogFactorial(space - mines, bestSpace - bestMines);
            relative.push_back(std::exp(log));
        }

        double sum = std::accumulate(relative.begin(), relative.end(), 0.0);
        std::vector<double> probability;
        probability.reserve(variants.size());

        for (double x: relative) {
            probability.push_back(x / sum);
        }

        return probability;
    }

    std::vector<double>
    getMineProbability(const std::vector<Variant> &variants, const std::vector<double> &variantProbability) {
        std::vector<double> mineProbability(variants.back().variables.size() + 1);
        for (int i = 0; i < variants.size(); ++i) {
            for (int j = 0; j < variants[i].variables.size(); ++j) {
                mineProbability[j] += variants[i].variables[j] * variantProbability[i];
            }
        }

        for (int i = 0; i < variants.size(); ++i) {
            int space = variants[i].leftSpace;
            int mines = variants[i].leftMines;
            mineProbability.back() += variantProbability[i] * (double) mines / space;
        }

        return mineProbability;
    }
}