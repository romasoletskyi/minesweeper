#include <cstdint>

namespace game {
    const static int HEIGHT = 16;//16;
    const static int WIDTH = 30;//30;
    const static int MINES = 99;//99;

    const static int BOMB = 11;
    const static int EMPTY = 2;
    const static int FLAG = 1;

// 0 - unknown, 1 - mine, n + 2 - open (n mines surrounding), 11 - true mine

    using State = std::array<std::array<uint8_t, WIDTH>, HEIGHT>;

    inline bool isOpened(const State &state, int i, int j) {
        return state[i][j] >= EMPTY;
    }

    inline bool isFlagged(const State &state, int i, int j) {
        return state[i][j] == FLAG;
    }

    inline int getMineCount(const State &state, int i, int j) {
        return state[i][j] - EMPTY;
    }

    inline bool isNeighbor(const State &state, int i, int j) {
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

template<>
struct std::hash<game::State> {
    size_t operator()(const game::State &s) const noexcept {
        size_t seed = 0;
        for (int i = 0; i < game::HEIGHT; ++i) {
            for (int j = 0; j < game::WIDTH; ++j) {
                seed ^= s[i][j] + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
        }
        return seed;
    }
};
