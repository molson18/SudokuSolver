#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "include/sudoku.h"

static PyObject *
SudokuSolver_solve(PyObject *self, PyObject *args) {
    Sudoku board;
    
}

static PyMethodDef SudokuSolverMethods[] = {
        {"solve", SudokuSolver_solve, METH_VARARGS, "Solves a Sudoku board"},
        {NULL, NULL, 0, NULL}
};

static struct PyModuleDef SudokuSolvermodule = {
        PyModuleDef_HEAD_INIT,
        "SudokuSolver",
        NULL,
        SudokuSolverMethods
};

PyMODINIT_FUNC
PyInit_SudokuSolver(void) {
    return PyModule_Create(&SudokuSolvermodule);
}

int main(int argc, char *argv[]) {
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    /* Add a built-in module, before Py_Initialize */
    if (PyImport_AppendInittab("SudokuSolver", PyInit_SudokuSolver) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        exit(1);
    }

    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(program);

    /* Initialize the Python interpreter.  Required.
       If this step fails, it will be a fatal error. */
    Py_Initialize();

    /* Optionally import the module; alternatively,
       import can be deferred until the embedded script
       imports it. */
    pmodule = PyImport_ImportModule("SudokuSolver");
    if (!pmodule) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'SudokuSolver'\n");
    }

    PyMem_RawFree(program);
    return 0;
}



