#include "minesweeper.h"

namespace py = pybind11;
using namespace game;

State readState(py::array_t<uint8_t, py::array::c_style | py::array::forcecast> state) {
    State s;
    uint8_t *sptr = static_cast<uint8_t *>(state.request().ptr);
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            s[i][j] = sptr[i * WIDTH + j];
        }
    }
    return s;
}

void writeStateToPtr(const State &state, uint8_t *ptr) {
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            ptr[i * WIDTH + j] = state[i][j];
        }
    }
}

py::array_t<uint8_t> writeState(const State &state) {
    auto result = py::array_t<uint8_t>({HEIGHT, WIDTH});
    writeStateToPtr(state, static_cast<uint8_t *>(result.request().ptr));
    return result;
}

std::vector<Action> readActions(py::array_t<int, py::array::c_style | py::array::forcecast> actions) {
    int *ptr = static_cast<int *>(actions.request().ptr);
    int size = actions.request().size / 3;

    std::vector<Action> a;
    a.reserve(size);
    for (int i = 0; i < size; ++i) {
        a.emplace_back(Action{ptr[3 * i], ptr[3 * i + 1], game::Cell(ptr[3 * i + 2])});
    }

    return a;
}

py::array_t<int> writeActions(const std::vector<Action> &actions) {
    auto result = py::array_t<int>({(int) actions.size(), 3});
    int *ptr = static_cast<int *>(result.request().ptr);

    for (int i = 0; i < actions.size(); ++i) {
        ptr[3 * i] = actions[i].i;
        ptr[3 * i + 1] = actions[i].j;
        ptr[3 * i + 2] = actions[i].cell;
    }

    return result;
}

std::vector<double>
readPolicy(const State &state, float *ptr) {
    auto actions = getPossibleActions(state);
    std::vector<double> p;

    for (auto action: actions) {
        p.push_back(ptr[action.i * WIDTH + action.j]);
    }

    return p;
}

void PyTreeAgent::loadState(py::array_t<uint8_t, py::array::c_style | py::array::forcecast> state,
                            py::array_t<float, py::array::c_style | py::array::forcecast> policy) {
    auto s = readState(state);
    auto p = readPolicy(s, static_cast<float *>(policy.request().ptr));
    agent_.loadState(std::move(s), std::move(p));
}

py::array_t<int> PyTreeAgent::getActions(py::array_t<uint8_t, py::array::c_style | py::array::forcecast> state) {
    return writeActions(agent_.getActions(readState(state)));
}

std::optional<py::array_t<uint8_t>> PyTreeAgent::explore() {
    auto state = agent_.explore();
    if (!state.has_value()) {
        return std::nullopt;
    }
    return writeState(state.value());
}

void PyTreeAgent::update(py::array_t<float, py::array::c_style | py::array::forcecast> policy, float value) {
    agent_.update(readPolicy(agent_.getState(), static_cast<float *>(policy.request().ptr)), value);
}

PyTreeManager::PyTreeManager(int batchSize, int treeIter) : batchSize_(batchSize), treeIter_(treeIter) {
    for (int i = 0; i < batchSize; ++i) {
        initializeAgent(i);
    }
}

py::array_t<uint8_t> PyTreeManager::getBatch() {
    py::array_t<uint8_t> result({batchSize_, HEIGHT, WIDTH});
    auto ptr = static_cast<uint8_t *>(result.request().ptr);

    for (int i = 0; i < batchSize_; ++i) {
        while (i == states_.size()) {
            if (free_.empty()) {
                initializeAgent(agents_.size());
            } else {
                int free = free_.front();
                free_.pop_front();

                if (!(agents_[free].getIter().has_value() && agents_[free].getIter().value() < treeIter_) &&
                    play(free)) {
                    initializeAgent(free);
                } else {
                    auto state = agents_[free].explore();
                    if (state.has_value()) {
                        states_.emplace_back(std::move(state.value()));
                    }
                }
            }
        }
        writeStateToPtr(states_[i], ptr + i * HEIGHT * WIDTH);
    }

    return result;
}

void PyTreeManager::loadBatch(py::array_t<float, py::array::c_style | py::array::forcecast> policy,
                              py::array_t<float, py::array::c_style | py::array::forcecast> values) {
    float *pptr = static_cast<float *>(policy.request().ptr);
    float *vptr = static_cast<float *>(values.request().ptr);

    for (int i = 0; i < batchSize_; ++i) {
        auto state = std::move(states_.front());
        auto job = std::move(jobs_.front());
        states_.pop_front();
        jobs_.pop_front();

        auto ptr = pptr + i * HEIGHT * WIDTH;
        if (job.task == Task::Initialize) {
            agents_[job.index].loadState(state, readPolicy(state, ptr));
        } else {
            agents_[job.index].update(readPolicy(state, ptr), vptr[i]);
        }

        free_.push_back(job.index);
    }
}

void PyTreeManager::initializeAgent(int index) {
    while (agents_.size() <= index) {
        agents_.emplace_back(gen_);
        boards_.emplace_back(gen_);
    }

    agents_[index] = agent::TreeAgent(gen_);
    boards_[index] = LoggingBoard(gen_);

    jobs_.emplace_back(Job{index, Task::Initialize});
    states_.emplace_back(boards_[index].getState());
}

bool PyTreeManager::play(int index) {
    while (true) {
        auto actions = agents_[index].getActions(boards_[index].getState());
        if (actions.empty()) {
            return false;
        }

        for (auto action: actions) {
            auto result = boards_[index].act(action);
            if (isTerminal(result)) {
                log_.emplace_back(std::move(boards_[index].getLogs()));
                return true;
            }
        }
    }
}

PYBIND11_MODULE(engine, m) {
    py::class_<PyTreeAgent>(m, "TreeAgent")
            .def(py::init<>())
            .def("load_state", &PyTreeAgent::loadState)
            .def("get_actions", &PyTreeAgent::getActions)
            .def("explore", &PyTreeAgent::explore)
            .def("update", &PyTreeAgent::update);

    py::class_<PyTreeManager>(m, "TreeManager")
            .def(py::init<>([](int batchSize, int treeIter) {
                return PyTreeManager(batchSize, treeIter);
            }))
            .def("get_batch", &PyTreeManager::getBatch)
            .def("load_batch", &PyTreeManager::loadBatch);
}
