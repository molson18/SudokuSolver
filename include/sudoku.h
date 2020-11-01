#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "Stack.h"

typedef struct Sudoku {
	char b[9][9];
} Sudoku;

typedef struct SudokuBucket_m {
	Sudoku board;
	struct SudokuBucket_m *next;
} SudokuBucket;

typedef struct SudokuSet_m {
	SudokuBucket **set;
	int buckets;
	int stored;
} SudokuSet;

typedef struct Move_m {
	char i;
	char j;
	char val;
} Move;

void printSudoku(Sudoku *board);
void readBoard(Sudoku *board, char *filename);
bool* getHints(Sudoku *board);

bool* hintsFromRow(Sudoku *board, int row);
bool* hintsFromCol(Sudoku *board, int col);
bool* hintsFromBox(Sudoku *board, int row, int col);

void solveBoard(Sudoku *board);
bool trivialMoves(Sudoku *board, SudokuSet *seen, Stack *moves);
bool checkRCB(Sudoku *board, bool *hints, int num, SudokuSet *seen, Stack *moves);
void insertMove(Sudoku *board, char move, int row, int col, bool *hints, Stack *moves);

void undoStackMoves(Sudoku *board, Stack *moves);

bool sudokuEquals(Sudoku *a, Sudoku *b);

SudokuSet *initSet(int numBuckets);
void deleteSet(SudokuSet *set);
int deleteBucket(SudokuBucket *bucket);

uint64_t sudokuHash(Sudoku *board);
bool inSudokuSet(SudokuSet *boardSet, Sudoku *board);
bool insertSudokuSet(SudokuSet *boardSet, Sudoku *board);
void resizeSudokuSet(SudokuSet *boardSet);
bool insertIntoBucket(SudokuBucket *bucket, Sudoku *board);

bool solvedBoard(Sudoku *board);
Stack *possibleMoves(Sudoku *board);
