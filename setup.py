from distutils.core import setup, Extension

module1 = Extension('SudokuSolver',
                            sources = ['SudokuSolvermodule.c'],
                            include_dirs=['include'],
                            extra_objects = ['build/libSudoku.a'])

setup (name = 'SudokuSolver',
                version = '1.0',
                description = 'A csp based sudoku solver built in c',
                ext_modules = [module1])
