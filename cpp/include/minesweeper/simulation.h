#pragma once

#include <algorithm>
#include <random>
#include <memory>

#include "mines.h"

namespace game {
    enum GameResult {
        Win,
        Lose,
        Continue
    };

    double getReward(GameResult result);

    GameResult getStateResult(const State& state);

    bool isTerminal(GameResult result);

    enum Cell {
        Open,
        Flag
    };

    struct Action {
        int i, j;
        Cell cell;
    };

    std::vector<Action> getPossibleActions(const State& state);

    class Board {
    public:
        Board(std::mt19937& gen);

        Board(std::mt19937& gen, const State& start);

        GameResult act(Action action);

        const State &getState() const;

    private:
        void setMine(int i, int j);

        void setMineCount(int i, int j, int count);

        int countNeighborMines(int i, int j) const;

        void populateMines(std::vector<std::pair<int, int>> indices, int mines);

        void restart(int i, int j);

        void openNeighborhood(int i, int j);

        void openCell(int i, int j);

        GameResult checkWinCondition() const;

        GameResult open(int i, int j);

        void flag(int i, int j);

    private:
        State state_;
        State open_;
        std::mt19937& gen_;

        int openedCells_ = 0;
        bool clear_ = true;
    };

    class PerfectBoard {
    public:
        PerfectBoard(std::mt19937& gen);

        PerfectBoard(std::mt19937& gen, const State& state);

        GameResult act(Action action);

        const State &getState() const;

    private:
        Board board_;
    };
}
