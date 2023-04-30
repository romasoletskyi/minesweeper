import browser
import engine

from time import sleep


def main():
    driver = browser.initialize()
    agent = engine.ExactAgent()

    while True:
        lost, _ = browser.get_state(driver).get_state()
        if not lost:
            browser.perfect_play(driver, agent)
        else:
            print("game over")

        sleep(1)


if __name__ == "__main__":
    main()
