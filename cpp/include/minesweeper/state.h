#include <cstdint>

namespace game {
    const static int HEIGHT = 16;//16;
    const static int WIDTH = 16;//30;
    const static int MINES = 40;//99;

    const static int BOMB = 11;
    const static int EMPTY = 2;
    const static int FLAG = 1;

// 0 - unknown, 1 - mine, n + 2 - open (n mines surrounding), 11 - true mine

    using State = std::array<std::array<uint8_t, WIDTH>, HEIGHT>;

    bool isOpened(const State &state, int i, int j);

    bool isFlagged(const State &state, int i, int j);

    int getMineCount(const State &state, int i, int j);

    bool isNeighbor(const State &state, int i, int j);
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
