#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "Sudoku.h"

static PyObject *
SudokuSolver_solve(PyObject *self, PyObject *args) 
{
    Sudoku board;
	
	PyObject * boardList;

	if (! PyArg_ParseTuple( args, "O!", &PyList_Type, &boardList )) 
	{
		return NULL;	
	}

	if (PyList_Size(boardList) != 81) 
	{
		fprintf(stderr, "Invalid board size, must have length of 81\n");
		return NULL;
	}

	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			PyObject * index = PyList_GetItem(boardList, 9 * i + j);
			long val = PyLong_AsLong(index);
			if (val < 0) {
				fprintf(stderr, "Invalid value at (%i, %i)\n", i, j);
				return NULL;
			}
			board.b[i][j] = val;
		}
	}

	solveBoard(&board);

	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			PyList_SET_ITEM(boardList, 9 * i + j, PyLong_FromLong(board.b[i][j]));
		}
	}
	return boardList;
}

static PyMethodDef SudokuSolverMethods[] = {
        {"solve", SudokuSolver_solve, METH_VARARGS, "Solves a Sudoku board"},
        {NULL, NULL, 0, NULL}
};

static struct PyModuleDef SudokuSolvermodule = {
        PyModuleDef_HEAD_INIT,
        "SudokuSolver",
		NULL,
        -1,
        SudokuSolverMethods
};

PyMODINIT_FUNC
PyInit_SudokuSolver(void) 
{
	PyMODINIT_FUNC test = PyModule_Create(&SudokuSolvermodule);
    return test; 
}

int 
main(int argc, char *argv[]) 
{
	wchar_t *program = Py_DecodeLocale(argv[0], NULL);
	if (program == NULL) {
		fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
		exit(1);
	}

	// Add a built-in module, before Py_Initialize <]
	if (PyImport_AppendInittab("SudokuSolver", PyInit_SudokuSolver) == -1) {
		fprintf(stderr, "Error: could not extend in-built modules table\n");
		exit(1);
	}

	// Pass argv[0] to the Python interpreter <]
	Py_SetProgramName(program);

	/* Initialize the Python interpreter.  Required.
	   If this step fails, it will be a fatal error. */
	Py_Initialize();

	/* Optionally import the module; alternatively,
	   import can be deferred until the embedded script
	   imports it. */
	PyObject *pmodule = PyImport_ImportModule("SudokuSolver");
	if (!pmodule) {
		PyErr_Print();
		fprintf(stderr, "Error: could not import module 'SudokuSolver'\n");
	}

	/*PyMem_RawFree(program);*/
	return 0;
}



