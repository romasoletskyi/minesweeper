#pragma once

#include <cstddef>
#include <utility>
#include <vector>
#include <unordered_map>

#include "state.h"

namespace game {
    struct Group {
        std::vector<int> indices;
        int mines = 0;
    };

    struct Constraint {
        std::vector<Group> groups;
        std::vector<std::pair<int, int>> coordinates;
    };

    Constraint getMineConstraints(const State &state);

    std::vector<Constraint> decoupleMineConstraints(const Constraint& constraint);

    struct Variant {
        std::vector<bool> variables;
    };

    std::vector<Variant> getMineVariants(const std::vector<Group> &constraints);

    std::vector<double> getPositionScore(const std::vector<int> &leftMines, int leftSpace);

    struct StateAnalysis {
        struct VariantGroup {
            std::vector<int> indices;
            int mines;
        };

        State state;

        std::vector<std::vector<std::pair<int, int>>> coordinates;
        std::vector<std::vector<Variant>> variants;
        std::vector<std::vector<VariantGroup>> condensedVariants;

        std::vector<std::vector<int>> condensedVariantsIndices;
        std::vector<double> condensedVariantsProbability;
    };

    StateAnalysis analyzeState(const State& state);
}