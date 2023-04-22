import numpy as np

HEIGHT = 16
WIDTH = 30


class State:
    def __init__(self):
        self.state = np.zeros((HEIGHT, WIDTH), dtype=int)
        self.lost = False

    def set_cell(self, i, j, value):
        self.state[i, j] = value + 2

    def set_mine(self, i, j):
        self.state[i, j] = 1

    def set_lost(self):
        self.lost = True

    def get_state(self):
        return self.lost, self.state
