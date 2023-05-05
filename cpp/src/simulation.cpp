#include "simulation.h"
#include "agent.h"

namespace game {
    double getReward(GameResult result) {
        if (result == GameResult::Win) {
            return 1;
        } else if (result == GameResult::Lose) {
            return -1;
        }
        return 0;
    }

    GameResult getStateResult(const State &state) {
        int open = 0;
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                if (state[i][j] == BOMB) {
                    return GameResult::Lose;
                }
                if (isOpened(state, i, j)) {
                    ++open;
                }
            }
        }
        if (open == HEIGHT * WIDTH - MINES) {
            return GameResult::Win;
        }
        return GameResult::Continue;
    }

    bool isTerminal(GameResult result) {
        return result != GameResult::Continue;
    }

    std::vector<Action> getPossibleActions(const State &state) {
        std::vector<Action> actions;
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                if (!isOpened(state, i, j) && !isFlagged(state, i, j)) {
                    actions.emplace_back(Action{i, j, Cell::Open});
                }
            }
        }
        return actions;
    }

    Board::Board(std::mt19937 &gen) : gen_(gen) {
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                open_[i][j] = 0;
                state_[i][j] = 0;
            }
        }
    }

    Board::Board(std::mt19937 &gen, const State &start) : gen_(gen), clear_(false) {
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                if (isOpened(start, i, j)) {
                    ++openedCells_;
                }
                open_[i][j] = start[i][j];
                state_[i][j] = 0;
            }
        }

        auto boundary = getBoundary(start);
        auto constraints = getMineConstraints(start, boundary);
        auto variants = getMineVariants(start, constraints);
        auto probability = getVariantProbability(start, variants);

        std::vector<double> cumulative(probability.size());
        std::partial_sum(probability.begin(), probability.end(), cumulative.begin());
        int chosen = static_cast<int>(std::distance(cumulative.begin(),
                                                    std::lower_bound(cumulative.begin(), cumulative.end(),
                                                                     std::uniform_real_distribution<>()(gen_))));

        auto boundaryIndices = boundary.getCoordinates();
        for (int n = 0; n < boundaryIndices.size(); ++n) {
            auto [i, j] = boundaryIndices[n];
            if (variants[chosen].variables[n]) {
                state_[i][j] = BOMB;
            }
        }

        std::vector<std::pair<int, int>> indices;
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                if (isFlagged(start, i, j)) {
                    state_[i][j] = BOMB;
                } else if (!isOpened(start, i, j) && !isNeighbor(start, i, j)) {
                    indices.emplace_back(i, j);
                }
            }
        }

        populateMines(indices, variants[chosen].leftMines);
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

    void Board::populateMines(std::vector<std::pair<int, int>> indices, int mines) {
        std::shuffle(indices.begin(), indices.end(), gen_);

        for (int n = 0; n < mines; ++n) {
            auto [i, j] = indices[n];
            setMine(i, j);
        }

        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                if (state_[i][j] != BOMB) {
                    setMineCount(i, j, countNeighborMines(i, j));
                }
            }
        }
    }

    void Board::restart(int i, int j) {
        std::vector<std::pair<int, int>> indices;

        for (int n = 0; n < HEIGHT; ++n) {
            for (int m = 0; m < WIDTH; ++m) {
                indices.emplace_back(n, m);
            }
        }

        std::swap(indices[i * WIDTH + j], indices.back());
        indices.pop_back();
        populateMines(std::move(indices), MINES);
    }

    GameResult Board::open(int i, int j) {
        if (clear_) {
            restart(i, j);
            clear_ = false;
        }

        if (state_[i][j] == BOMB) {
            open_[i][j] = state_[i][j];
            return GameResult::Lose;
        }

        if (isOpened(open_, i, j)) {
            int flags = 0;
            int mines = 0;
            bool incorrect = false;

            for (int n = std::max(i - 1, 0); n < std::min(i + 2, HEIGHT); ++n) {
                for (int m = std::max(j - 1, 0); m < std::min(j + 2, WIDTH); ++m) {
                    if (state_[n][m] == BOMB && !isFlagged(open_, n, m)) {
                        incorrect = true;
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
                if (incorrect) {
                    return GameResult::Lose;
                }
            }
            return checkWinCondition();
        }

        if (state_[i][j] == EMPTY) {
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

    GameResult Board::act(Action action) {
        if (action.cell == Cell::Open) {
            return open(action.i, action.j);
        } else {
            flag(action.i, action.j);
            return GameResult::Continue;
        }
    }

    PerfectBoard::PerfectBoard(std::mt19937 &gen) : board_(gen) {
    }

    PerfectBoard::PerfectBoard(std::mt19937 &gen, const State& state) : board_(gen, state) {
    }

    const State &PerfectBoard::getState() const {
        return board_.getState();
    }

    GameResult PerfectBoard::act(Action action) {
        auto result = board_.act(action);
        if (isTerminal(result)) {
            return result;
        }

        while (true) {
            auto actions = agent::getExactActions(getState());
            if (actions.empty()) {
                break;
            }

            for (const auto &a: actions) {
                result = board_.act(a);
                if (isTerminal(result)) {
                    return result;
                }
            }
        }

        return GameResult::Continue;
    }
}