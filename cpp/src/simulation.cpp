#include "simulation.h"

Board::Board() {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            open_[i][j] = 0;
        }
    }
}

void Board::setMine(int i, int j) {
    state_[i][j] = BOMB;
}

void Board::setMineCount(int i, int j, int count) {
    state_[i][j] = EMPTY + count;
}


int Board::countNeighborMines(int i, int j) const {
    int mines = 0;
    for (int n = std::max(i - 1, 0); n < std::min(i + 2, HEIGHT); ++n) {
        for (int m = std::max(j - 1, 0); m < std::min(j + 2, WIDTH); ++m) {
            if (state_[n][m] == BOMB) {
                ++mines;
            }
        }
    }
    return mines;
}

void Board::restart(int i, int j) {
    std::vector<std::pair<int, int>> indices;
    for (int n = 0; n < HEIGHT; ++n) {
        for (int m = 0; m < WIDTH; ++m) {
            indices.emplace_back(n, m);
        }
    }

    std::shuffle(indices.begin(), indices.end(), gen_);

    int start;
    for (int n = 0; n < indices.size(); ++n) {
        if (indices[n] == std::make_pair(i, j)) {
            start = n;
        }
    }

    std::swap(indices[start], indices.back());

    for (int n = 0; n < MINES; ++n) {
        auto [a, b] = indices[n];
        setMine(a, b);
    }

    for (int n = MINES; n < indices.size(); ++n) {
        auto [a, b] = indices[n];
        setMineCount(a, b, countNeighborMines(a, b));
    }
}

GameResult Board::open(int i, int j) {
    if (clear_) {
        restart(i, j);
        clear_ = false;
    }

    if (state_[i][j] == BOMB) {
        return GameResult::Lose;
    }

    if (isOpened(open_, i, j)) {
        int flags = 0;
        int mines = 0;
        for (int n = std::max(i - 1, 0); n < std::min(i + 2, HEIGHT); ++n) {
            for (int m = std::max(j - 1, 0); m < std::min(j + 2, WIDTH); ++m) {
                if (state_[n][m] == BOMB && !isFlagged(open_, n, m)) {
                    return GameResult::Lose;
                }
                if (state_[n][m] == BOMB) {
                    ++mines;
                }
                if (isFlagged(open_, n, m)) {
                    ++flags;
                }
            }
        }
        if (flags == mines) {
            openNeighborhood(i, j);
        }
        return checkWinCondition();
    }

    if (open_[i][j] == EMPTY) {
        openNeighborhood(i, j);
    } else {
        openCell(i, j);
    }

    return checkWinCondition();
}

void Board::openNeighborhood(int i, int j) {
    openCell(i, j);
    for (int n = std::max(i - 1, 0); n < std::min(i + 2, HEIGHT); ++n) {
        for (int m = std::max(j - 1, 0); m < std::min(j + 2, WIDTH); ++m) {
            if (!isOpened(open_, n, m) && state_[n][m] == EMPTY) {
                openNeighborhood(n, m);
            } else if (state_[n][m] != BOMB) {
                openCell(n, m);
            }
        }
    }
}

void Board::openCell(int i, int j) {
    if (!isOpened(open_, i, j)) {
        ++openedCells_;
    }
    open_[i][j] = state_[i][j];
}

void Board::flag(int i, int j) {
    if (!isOpened(open_, i, j)) {
        open_[i][j] = FLAG;
    }
}

const State &Board::getState() const {
    return open_;
}

GameResult Board::checkWinCondition() const {
    if (openedCells_ == WIDTH * HEIGHT - MINES) {
        return GameResult::Win;
    }
    return GameResult::Continue;
}
