import numpy as np

from bs4 import BeautifulSoup
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver import ActionChains
from selenium.webdriver.support.wait import WebDriverWait
from selenium.webdriver.support.expected_conditions import presence_of_element_located
from time import sleep

from state import State


def initialize():
    driver = webdriver.Firefox()
    driver.get("https://minesweeper.online/start/3")
    return driver


def get_state(driver):
    canvas = WebDriverWait(driver, 20).until(presence_of_element_located((By.ID, "A43"))).get_attribute("innerHTML")
    soup = BeautifulSoup(canvas, "html.parser")
    state = State()

    for cell in soup.find_all("div", {"class": "cell"}):
        x, y = cell["id"][5:].split("_")
        if "hd_opened" in cell["class"]:
            value = int(cell["class"][-1][7:])
            state.set_cell(int(y), int(x), value)
        if "hd_flag" in cell["class"]:
            state.set_mine(int(y), int(x))

    if "hd_top-area-face-lose" in driver.find_element(By.ID, "top_area_face").get_attribute("class").split():
        state.set_lost()

    return state


def open_cell(driver, i, j):
    driver.find_element(By.ID, "cell_{0}_{1}".format(j, i)).click()


def set_mine(driver, i, j):
    action = ActionChains(driver)
    action.context_click(driver.find_element(By.ID, "cell_{0}_{1}".format(j, i))).perform()


def perfect_play(driver, agent):
    while True:
        _, state = get_state(driver).get_state()
        print(state)
        agent.loadState(state)
        actions = agent.getActions()

        if len(actions) > 0:
            print(actions)
            for action in actions:
                cell, i, j = action
                if cell == 0:
                    open_cell(driver, i, j)
                else:
                    if state[i, j] == 0:
                        set_mine(driver, i, j)
                sleep(0.1 + 0.2 * np.random.random())
        else:
            print("out of turns")
            break
