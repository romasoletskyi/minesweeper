from bs4 import BeautifulSoup
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver import ActionChains

from state import State


def initialize():
    driver = webdriver.Firefox()
    driver.get("https://minesweeper.online/start/3")
    return driver


def get_state(driver):
    canvas = driver.find_element(By.ID, "A43").get_attribute("innerHTML")
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
