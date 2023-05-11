#include <deque>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "agent.h"

namespace py = pybind11;
using namespace game;

class PyTreeAgent {
public:
    explicit PyTreeAgent() : agent_(gen_) {}

    void loadState(py::array_t<uint8_t, py::array::c_style | py::array::forcecast> state,
                   py::array_t<float, py::array::c_style | py::array::forcecast> policy);

    py::array_t<int> getActions(py::array_t<uint8_t, py::array::c_style | py::array::forcecast> state);

    std::optional<py::array_t<uint8_t>> explore();

    void update(py::array_t<float, py::array::c_style | py::array::forcecast> policy, float value);

private:
    std::mt19937 gen_;
    agent::TreeAgent agent_;
};

enum Task {
    Initialize,
    Update
};

struct Job {
    int index;
    Task task;
};

struct GameLog {
    std::vector<State> states;
    std::vector<Action> actions;
    double result;
};

class LoggingBoard {
public:
    explicit LoggingBoard(std::mt19937 &gen) : board_(gen) {}

    LoggingBoard(const LoggingBoard &board) : board_(board.board_), log_(board.log_) {}

    LoggingBoard &operator=(const LoggingBoard &board) {
        *this = LoggingBoard(board);
        return *this;
    }

    LoggingBoard(LoggingBoard &&board) : board_(std::move(board.board_)), log_(std::move(board.log_)) {}

    LoggingBoard &operator=(LoggingBoard &&board) {
        board_ = std::move(board.board_);
        log_ = std::move(board.log_);
        return *this;
    }

    GameResult act(Action action) {
        log_.states.emplace_back(board_.getState());
        log_.actions.emplace_back(action);

        auto result = board_.act(action);
        if (isTerminal(result)) {
            log_.result = getReward(result);
        }

        return result;
    }

    GameLog getLogs() {
        return std::move(log_);
    }

    const State &getState() const {
        return board_.getState();
    }

private:
    Board board_;
    GameLog log_;
};

class PyTreeManager {
public:
    PyTreeManager(int batchSize, int treeIter);

    py::array_t<uint8_t> getBatch();

    void loadBatch(py::array_t<float, py::array::c_style | py::array::forcecast> policy,
                   py::array_t<float, py::array::c_style | py::array::forcecast> values);

private:
    void initializeAgent(int index);

    bool play(int index);

private:
    std::mt19937 gen_;
    std::vector<agent::TreeAgent> agents_;
    std::vector<LoggingBoard> boards_;

    std::deque<GameLog> log_;
    std::deque<State> states_;
    std::deque<Job> jobs_;
    std::deque<int> free_;

    int batchSize_;
    int treeIter_;
};