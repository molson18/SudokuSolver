import SudokuSolver
import time
from selenium import webdriver
from selenium.webdriver.common.keys import Keys

DRIVER_PATH = "./chromedriver"

if __name__ == "__main__":
    print("Hello World")
    test = [0, 3, 1, 0, 0, 0, 7, 0, 8, 0, 4, 0, 5, 8, 0, 0, 1, 9, 8, 0, 5, 1, 0, 9, 0, 0, 4, 0, 0, 2, 8, 0, 1, 0, 6, 0, 6, 0, 0, 7, 0, 0, 8, 2, 0, 0, 9, 0, 0, 2, 6, 0, 0, 1, 0, 8, 0, 0, 5, 0, 3, 4, 0, 2, 5, 0, 0, 3, 8, 0, 0, 0, 4, 0, 3, 0, 0, 2, 0, 8, 0]
    test = [0, 7, 0, 8, 0, 0, 0, 0, 2, 0, 0, 0, 0, 6, 9, 0, 0, 0, 1, 0, 5, 0, 0, 0, 0, 0, 8, 0, 0, 1, 9, 0, 0, 0, 6, 0, 0, 0, 8, 0, 4, 0, 2, 0, 0, 0, 6, 0, 0, 0, 7, 9, 0, 0, 6, 0, 0, 0, 0, 0, 3, 0, 4, 0, 0, 0, 6, 7, 0, 0, 0, 0, 4, 0, 0, 0, 0, 1, 0, 5, 0]
#    new = SudokuSolver.solve(test)
    print(test)
#    print(new)
#    if test == new:
#        print("They're the same object")
    driver = webdriver.Chrome(executable_path=DRIVER_PATH)
    driver.get('https://www.websudoku.com/?level=4')
    
    frame = driver.find_element_by_xpath('//frame')
    driver.switch_to.frame(frame)

    driver.maximize_window()
    puz = driver.find_element_by_id('puzzle_grid')

    board_pieces = puz.find_elements_by_tag_name('input')
    print(len(board_pieces))

    board = []
    for piece in board_pieces:
        val = piece.get_attribute("value")
        if val == "": 
            board.append(0)
        else:
            board.append(int(val))

    board_pieces[0].click()

    print(board);
    new = SudokuSolver.solve(board)
    print(new)
    print(board)
    for i in range(81):
        board_pieces[i].send_keys(board[i])
    print("Idk anymore")
    time.sleep(51)
    driver.find_element_by_name("submit").click()
    time.sleep(10)
    driver.close()

