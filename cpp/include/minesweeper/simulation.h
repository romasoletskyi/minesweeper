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

    GameResult getStateResult(const State &state);

    bool isTerminal(GameResult result);

    enum Cell {
        Open,
        Flag
    };

    struct Action {
        int i, j;
        Cell cell;
    };

    std::vector<Action> getPossibleActions(const State &state);

    class Board {
    public:
        explicit Board(std::mt19937 &gen);

        Board(const Board &board) : state_(board.state_), open_(board.open_), gen_(board.gen_),
                                    openedCells_(board.openedCells_), clear_(board.clear_) {
        }

        Board &operator=(const Board &board) {
            *this = Board(board);
            return *this;
        }

        Board(Board &&board) : state_(std::move(board.state_)), open_(std::move(board.open_)), gen_(board.gen_),
                               openedCells_(board.openedCells_), clear_(board.clear_) {
        }

        Board &operator=(Board &&board) {
            state_ = std::move(board.state_);
            open_ = std::move(board.open_);
            gen_ = board.gen_;
            openedCells_ = board.openedCells_;
            clear_ = board.clear_;
            return *this;
        }

        Board(std::mt19937 &gen, const StateAnalysis &analysis);

        GameResult act(Action action);

        const State &getState() const;

    private:
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
        std::mt19937 &gen_;

        int openedCells_ = 0;
        bool clear_ = true;
    };

    class PerfectBoard {
    public:
        explicit PerfectBoard(std::mt19937 &gen);

        PerfectBoard(std::mt19937 &gen, const StateAnalysis &analysis);

        GameResult act(Action action);

        const State &getState() const;

    private:
        Board board_;
    };
}
