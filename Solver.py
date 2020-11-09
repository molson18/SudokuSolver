import SudokuSolver

if __name__ == "__main__":
    print("Hello World")
    test = [0, 3, 1, 0, 0, 0, 7, 0, 8, 0, 4, 0, 5, 8, 0, 0, 1, 9, 8, 0, 5, 1, 0, 9, 0, 0, 4, 0, 0, 2, 8, 0, 1, 0, 6, 0, 6, 0, 0, 7, 0, 0, 8, 2, 0, 0, 9, 0, 0, 2, 6, 0, 0, 1, 0, 8, 0, 0, 5, 0, 3, 4, 0, 2, 5, 0, 0, 3, 8, 0, 0, 0, 4, 0, 3, 0, 0, 2, 0, 8, 0]
    new = SudokuSolver.solve(test)
    print(test)
    print(new)
    if test == new:
        print("They're the same object")
