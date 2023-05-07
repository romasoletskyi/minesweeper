#include <cmath>
#include <algorithm>
#include <numeric>
#include <optional>
#include <unordered_set>

#include "mines.h"

template<>
struct std::hash<std::pair<int, int>> {
    size_t operator()(const std::pair<int, int> &x) const noexcept {
        return std::hash<int64_t>()(static_cast<int64_t>(x.first) << 32 | x.second);
    }
};

namespace game {
    static const std::vector<std::pair<int, int>> offset = {{-1, -1},
                                                            {-1, 0},
                                                            {-1, 1},
                                                            {0,  -1},
                                                            {0,  1},
                                                            {1,  -1},
                                                            {1,  0},
                                                            {1,  1}};

    Constraint getMineConstraints(const State &state) {
        std::vector<Group> groups(HEIGHT * WIDTH);
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                if (state[i][j] > EMPTY) {
                    groups[i * WIDTH + j].mines = state[i][j] - EMPTY;
                }
            }
        }

        std::unordered_map<std::pair<int, int>, int> indices;

        for (auto [dRow, dColumn]: offset) {
            int startRow = std::max(-dRow, 0);
            int endRow = std::min(HEIGHT - dRow, HEIGHT);
            int startColumn = std::max(-dColumn, 0);
            int endColumn = std::min(WIDTH - dColumn, WIDTH);

            for (int i = startRow; i < endRow; ++i) {
                for (int j = startColumn; j < endColumn; ++j) {
                    if (state[i][j] >= EMPTY) {
                        if (state[i + dRow][j + dColumn] == FLAG) {
                            --groups[i * WIDTH + j].mines;
                        } else if (!state[i + dRow][j + dColumn]) {
                            auto pair = std::make_pair(i + dRow, j + dColumn);
                            if (indices.count(pair)) {
                                groups[i * WIDTH + j].indices.push_back(indices[pair]);
                            } else {
                                int index = static_cast<int>(indices.size());
                                groups[i * WIDTH + j].indices.push_back(index);
                                indices[pair] = index;
                            }
                        }
                    }
                }
            }
        }

        std::vector<Group> filteredGroups;
        for (const auto &group: groups) {
            if (!group.indices.empty()) {
                filteredGroups.push_back(group);
            }
        }

        std::vector<std::pair<int, int>> coordinates(indices.size());
        for (const auto &[pair, index]: indices) {
            coordinates[index] = pair;
        }

        return {filteredGroups, coordinates};
    }

    std::vector<Constraint> decoupleMineConstraints(const Constraint &constraint) {
        std::vector<std::vector<int>> edges(constraint.coordinates.size());
        std::vector<std::vector<int>> vertexToGroup(constraint.coordinates.size());

        for (int i = 0; i < constraint.groups.size(); ++i) {
            const auto &indices = constraint.groups[i].indices;
            for (int j = 1; j < indices.size(); ++j) {
                edges[indices[j]].push_back(indices[j - 1]);
                edges[indices[j - 1]].push_back(indices[j]);
            }
            for (int vertex: indices) {
                vertexToGroup[vertex].push_back(i);
            }
        }

        std::unordered_set<int> visited;
        std::vector<Constraint> constraints;

        for (int i = 0; i < constraint.coordinates.size(); ++i) {
            std::unordered_set<int> visitedGroups;
            std::unordered_map<int, int> visitedVertices;
            std::vector<int> vertices = {i};

            while (!vertices.empty()) {
                int vertex = vertices.back();
                vertices.pop_back();
                if (!visited.count(vertex)) {
                    visited.insert(vertex);
                    visitedVertices[vertex] = static_cast<int>(visitedVertices.size());
                    for (int neighbor: edges[vertex]) {
                        vertices.push_back(neighbor);
                    }
                }
            }

            for (auto [vertex, _]: visitedVertices) {
                for (int group: vertexToGroup[vertex]) {
                    visitedGroups.insert(group);
                }
            }

            Constraint c;
            c.coordinates.resize(visitedVertices.size());

            for (auto [vertex, vertexIndex]: visitedVertices) {
                c.coordinates[vertexIndex] = constraint.coordinates[vertex];
            }
            for (int group: visitedGroups) {
                Group g;
                g.mines = constraint.groups[group].mines;
                for (int vertex: constraint.groups[group].indices) {
                    g.indices.push_back(visitedVertices[vertex]);
                }
                c.groups.emplace_back(std::move(g));
            }

            if (!c.groups.empty()) {
                constraints.emplace_back(std::move(c));
            }
        }

        return constraints;
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

            groups.push_back(Group{indices, mines});
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
                  std::vector<Variant> &variants) {
        while (!constraints.empty()) {
            bool reduce = false;
            while (!constraints.empty()) {
                if (constraints.back().mines < 0 ||
                    constraints.back().mines > constraints.back().indices.size()) {
                    return;
                } else if (constraints.back().mines == 0 ||
                           constraints.back().mines == constraints.back().indices.size()) {
                    reduce = true;
                    bool set = constraints.back().mines;
                    for (int var: constraints.back().indices) {
                        if (setVariables.count(var)) {
                            if (setVariables.at(var) != set) {
                                return;
                            }
                        } else {
                            setVariables[var] = set;
                        }
                    }
                    constraints.pop_back();
                } else {
                    break;
                }
            }
            if (!reduce) {
                break;
            }
            constraints = reduceConstraints(constraints, setVariables);
        }
        if (constraints.empty()) {
            std::vector<bool> variant(setVariables.size());
            for (const auto &[index, variable]: setVariables) {
                variant[index] = variable;
            }
            variants.emplace_back(Variant{variant});
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
                    setMines(reduced, branchSetVariables, variants);
                }
            } while (!(++it).overflow());
        }
    }

    std::vector<Variant> getMineVariants(const std::vector<Group> &constraints) {
        std::vector<Variant> variants;
        std::unordered_map<int, bool> setVariables;
        setMines(reduceConstraints(constraints, setVariables), setVariables, variants);
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

    std::vector<double> getPositionScore(const std::vector<int> &leftMines, int leftSpace) {
        int bestMines = *std::max_element(leftMines.begin(), leftMines.end(),
                                          [leftSpace](const auto &lhs, const auto &rhs) {
                                              double lhsRate = (double) lhs / leftSpace;
                                              double rhsRate = (double) rhs / leftSpace;
                                              return std::abs(lhsRate - 0.5) > std::abs(rhsRate - 0.5);
                                          });
        std::vector<double> score;
        score.reserve(leftMines.size());

        for (int mines: leftMines) {
            if (mines < 0 || mines > leftSpace) {
                score.push_back(0);
            } else {
                score.push_back(std::exp(diffLogFactorial(bestMines, mines) +
                                         diffLogFactorial(leftSpace - bestMines, leftSpace - mines)));
            }
        }

        return score;
    }

    StateAnalysis analyzeState(const State &state) {
        StateAnalysis analysis = {state};

        auto constraints = decoupleMineConstraints(getMineConstraints(state));
        for (const auto &constraint: constraints) {
            analysis.coordinates.emplace_back(constraint.coordinates);
        }

        for (const auto &constraint: constraints) {
            analysis.variants.emplace_back(getMineVariants(constraint.groups));
        }

        for (const auto &coupledVariants: analysis.variants) {
            std::unordered_map<int, std::vector<int>> variantMines;
            for (int i = 0; i < coupledVariants.size(); ++i) {
                int mines = 0;
                for (int cell: coupledVariants[i].variables) {
                    mines += cell;
                }
                variantMines[mines].push_back(i);
            }

            std::vector<StateAnalysis::VariantGroup> variantGroups;
            for (const auto &[mines, group]: variantMines) {
                variantGroups.emplace_back(StateAnalysis::VariantGroup{group, mines});
            }
            analysis.condensedVariants.emplace_back(std::move(variantGroups));
        }

        int pivot = 0, size = static_cast<int>(analysis.condensedVariants.size());
        std::vector<int> index(size);

        int leftSpace = HEIGHT * WIDTH;
        int leftMines = MINES;
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                if (state[i][j]) {
                    if (state[i][j] == FLAG) {
                        --leftMines;
                    }
                    --leftSpace;
                }
            }
        }
        for (const auto &coordinates: analysis.coordinates) {
            leftSpace -= static_cast<int>(coordinates.size());
        }

        std::vector<int> leftCondensedVariantsMines;

        while (true) {
            int mines = 0;
            for (int i = 0; i < size; ++i) {
                mines += analysis.condensedVariants[i][index[i]].mines;
            }
            leftCondensedVariantsMines.push_back(leftMines - mines);

            analysis.condensedVariantsIndices.push_back(index);
            while (pivot < size && ++index[pivot] == analysis.condensedVariants[pivot].size()) {
                index[pivot++] = 0;
            }
            if (pivot == size) {
                break;
            }
            pivot = 0;
        }

        auto score = getPositionScore(leftCondensedVariantsMines, leftSpace);
        for (int i = 0; i < score.size(); ++i) {
            for (int j = 0; j < size; ++j) {
                score[i] *= static_cast<double>(analysis.condensedVariants[j][analysis.condensedVariantsIndices[i][j]].indices.size());
            }
        }

        double scoreSum = std::accumulate(score.begin(), score.end(), 0.0);
        for (double &s: score) {
            s /= scoreSum;
        }

        analysis.condensedVariantsProbability = std::move(score);
        return analysis;
    }
}