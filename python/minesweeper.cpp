#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "agent.h"

namespace py = pybind11;

class PyExactAgent {
public:
    void loadState(py::array_t<int, py::array::c_style | py::array::forcecast> state) {
        int *ptr = static_cast<int *>(state.request().ptr);

        State s;
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                s[i][j] = ptr[i * WIDTH + j];
            }
        }

        agent_.loadState(s);
    }

    py::array_t<int> getActions() const {
        auto actions = agent_.getActions();
        auto result = py::array_t<int>({(int) actions.size(), 3});
        int* ptr = static_cast<int *>(result.request().ptr);

        for (int i = 0; i < actions.size(); ++i) {
            ptr[3 * i] = actions[i].cell;
            ptr[3 * i + 1] = actions[i].i;
            ptr[3 * i + 2] = actions[i].j;
        }

        return result;
    }

private:
    ExactAgent agent_;
};

PYBIND11_MODULE(engine, m) {
    py::class_<PyExactAgent>(m, "ExactAgent")
            .def(py::init<>())
            .def("loadState", &PyExactAgent::loadState)
            .def("getActions", &PyExactAgent::getActions);
}
