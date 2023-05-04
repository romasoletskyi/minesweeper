#include <algorithm>

#include "state.h"

namespace game {
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
}