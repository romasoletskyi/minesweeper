#pragma once

#include "mines.h"

enum Cell {
    Free,
    Mine
};

struct Action {
    int i, j;
    Cell cell;
};

class ExactAgent {
public:
    void loadState(const State& state);

    std::vector<Action> getActions() const;

private:
    State state_;
};

class RandomAgent {
public:
    void loadState(const State& state);

    std::vector<Action> getActions() const;

private:
    ExactAgent agent_;
};