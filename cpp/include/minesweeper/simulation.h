#pragma once

#include <algorithm>
#include <random>
#include "mines.h"

enum GameResult {
    Win,
    Lose,
    Continue
};

class Board {
public:
    Board();

    GameResult open(int i, int j);

    void flag(int i, int j);

    const State& getState() const;

private:
    void setMine(int i, int j);

    void setMineCount(int i, int j, int count);

    int countNeighborMines(int i, int j) const;

    void restart(int i, int j);

    void openNeighborhood(int i, int j);

    void openCell(int i, int j);

    GameResult checkWinCondition() const;

private:
    State state_;
    State open_;
    std::mt19937 gen_;

    int openedCells_ = 0;
    bool clear_ = true;
};
