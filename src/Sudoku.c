#include "Sudoku.h"

/*int main(int argc, char* argv[]) {*/


	/*if (argc != 2) {*/
		/*fprintf(stderr, "Usage: ./Sudoku filename\n");*/
		/*return 1;*/
	/*}			*/

	/*Sudoku *board = malloc(sizeof(Sudoku));*/
	/*readBoard(board, argv[1]);*/

	/*printf("%llx\n", sudokuHash(board));*/
	/*printSudoku(board);*/

	/*clock_t start = clock();*/
	/*solveBoard(board);*/
	/*double time = 1000 * (double)(clock() - start) / CLOCKS_PER_SEC;*/
	/*printf("%f milliseconds\n", time);*/
	/*printSudoku(board);*/
	/*printf("%llx\n", sudokuHash(board));*/
	/*free(board);*/
/*}*/

void printSudoku(Sudoku *board) {
	printf("-------------------------\n");
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (j % 3 == 0) { printf("| "); }
			char temp = board->b[i][j];
			if (temp) { printf("%i ", temp); }
			else { printf("  "); }
		}
		printf("|\n");
		if ((i + 1) % 3 == 0 && i != 8) {
			printf("|-------+-------+-------|\n");
		}
	}
	printf("-------------------------\n");
}

void readBoard(Sudoku *board, char *filename) {
	FILE *fin = fopen(filename, "r");
	if (!fin) {
		fprintf(stderr, "Error: could not open file %s\n", filename);
		exit(2);
	}

	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			int temp = -1;

			if (!fscanf(fin, "%i ", &temp)) {
				fprintf(stderr, "Error: invalid file format\n");
				exit(3);
			}

			board->b[i][j] = temp;
		}
	}

	fclose(fin);
}

bool* getHints(Sudoku *board) {
	bool *hints = calloc(9 * 9 * 9, sizeof(bool));
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (board->b[i][j] == 0) {
				bool *rows = hintsFromRow(board, i);
				bool *cols = hintsFromCol(board, j);
				bool *box = hintsFromBox(board, i, j);

				for (int k = 0; k < 9; k++) {
					hints[81 * i + 9 * j + k] = !(rows[k] || cols[k] || box[k]);
				}

				free(rows);
				free(cols);
				free(box);
			}
		}
	}
	return hints;
}

bool* hintsFromRow(Sudoku *board, int row) {
	bool *rowHints = calloc(9, sizeof(bool));

	for (int i = 0; i < 9; i++) {
		int val = board->b[row][i];
		if (val) {
			rowHints[val - 1] = true;
		}
	}

	return rowHints;
}

bool* hintsFromCol(Sudoku *board, int col) {
	bool *colHints = calloc(9, sizeof(bool));

	for (int i = 0; i < 9; i++) {
		int val = board->b[i][col];
		if (val) {
			colHints[val - 1] = true;
		}
	}

	return colHints;
}

bool* hintsFromBox(Sudoku *board, int row, int col) {
	bool *box = calloc(9, sizeof(bool));

	for (int i = 0; i < 9; i++) {
		int val = board->b[3 * (row / 3) + (i / 3)][3 * (col / 3) + (i % 3)];
		if (val) {
			box[val - 1] = true;
		}
	}

	return box;
}

void solveBoard(Sudoku *board) {
	/*int change = 1;*/
	SudokuSet *seen = initSet(1);
	Stack *moves = initStack();
	Stack *temp = initStack();
	trivialMoves(board, seen, temp);
	deleteStack(temp);
	while(!solvedBoard(board)) {
		/*
		get possible moves
		try the first untried move
		 	if theres no untried moves, remove last move
		fill out trivialmoves of that move
		if unsolvable undo those moves
		if solvable push onto move stack
		*/
		Stack *possMoves = possibleMoves(board);
		while(possMoves->length) {
			Move *m = pop(possMoves);
			board->b[(int)m->i][(int)m->j] = m->val;
			if (insertSudokuSet(seen, board)) {
				Stack *ms = initStack();
				if (trivialMoves(board, seen, ms)) {
					push(ms, m);
					push(moves, ms);
					break;
				}
				undoStackMoves(board, ms);
			}
			board->b[(int)m->i][(int)m->j] = 0;
			free(m);
		}
		if (possMoves->length == 0) {
			/*printf("Ran out of moves\n");*/
			/*printSudoku(board);*/
			Stack *badMoves = pop(moves);
			if (!badMoves) {
				printf("Unsolvable Board\n");
				return;
			}
			undoStackMoves(board, badMoves);
		}
		deleteStack(possMoves);
	}
	while (moves->length) {
		Stack *deleteMoves = pop(moves);
		deleteStack(deleteMoves);
	}
	deleteStack(moves);
	deleteSet(seen);
}

void undoStackMoves(Sudoku *board, Stack *moves) {
	while(moves->length) {
		Move *undo = pop(moves);
		board->b[(int)undo->i][(int)undo->j] = 0;
		free (undo);
	}
	deleteStack(moves);
}

bool trivialMoves(Sudoku *board, SudokuSet *seen, Stack *moves) {
	int change = -1;
	bool *hints;
	while (change) {
		change = 0;
		hints = getHints(board);
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 9; j++) {
				// check if theres only one viable move at (i, j)
				if (board->b[i][j] == 0) {
					int lastMove = -1;
					int numMoves = 0;
					for (int k = 0; k < 9; k++) {
						if (hints[81 * i + 9 * j + k]) {
							lastMove = k;
							numMoves++;
						}
					}
					if (numMoves == 1) {
						change++;
						insertMove(board, lastMove + 1, i, j, hints, moves);
						insertSudokuSet(seen, board);
						/*printf("buckets: %i, seen: %i\n", seen->buckets, seen->stored);*/
					}
					else if (numMoves == 0) {
						/*printf("row: %i, col: %i, number: %i", i, j, lastMove + 1);*/
						free(hints);
						return false;
					}
				}
			}
			// check row/col/box i
			if (!checkRCB(board, hints, i, seen, moves)) {
				free(hints);
				return false;
			}
		}
		free(hints);
	}
	return true;
}

bool checkRCB(Sudoku *board, bool *hints, int num, SudokuSet *seen, Stack *moves) {
	char numInRow[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	char numInCol[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	char numInBox[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	char lastSeenInRow[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
	char lastSeenInCol[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
	char lastSeenInBox[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
	// in each rcb
	// 		at each value
	// 			for each possible value
	// 				incriment numInRow if true
	// 				set last seen to that value if true
	for (int i = 0; i < 9; i++) {
		int rowVal = board->b[num][i];
		int colVal = board->b[i][num];
		int boxRow = 3 * (num / 3) + i / 3;
		int boxCol = 3 * (num % 3) + i % 3;
		int boxVal = board->b[boxRow][boxCol];
		if (rowVal) { numInRow[rowVal - 1] = 9; }
		if (colVal) { numInCol[colVal - 1] = 9; }
		if (boxVal) { numInBox[boxVal - 1] = 9; }
		for (int j = 0; j < 9; j++) {
			bool row = hints[81 * num + 9 * i + j];
			bool col = hints[81 * i + 9 * num + j];
			int boxRow = 3 * (num / 3) + i / 3;
			int boxCol = 3 * (num % 3) + i % 3;
			bool box = hints[81 * boxRow + 9 * boxCol + j];
			if (row) {
				numInRow[j]++;
				lastSeenInRow[j] = i;
			}
			if (col) {
				numInCol[j]++;
				lastSeenInCol[j] = i;
			}
			if (box) {
				numInBox[j]++;
				lastSeenInBox[j] = i;
			}
		}
	}	
	for (int i = 0; i < 9; i++) {
		if (!(numInRow[i] && numInCol[i] && numInBox[i])) {
			return false;
		}
		if (numInRow[i] == 1) {
			insertMove(board, i + 1, num, lastSeenInRow[i], hints, moves);
			insertSudokuSet(seen, board);
		}	
		if (numInCol[i] == 1) {
			insertMove(board, i + 1, lastSeenInCol[i], num, hints, moves);
			insertSudokuSet(seen, board);
		}
		if (numInBox[i] == 1) {
			int boxRow = 3 * (num / 3) + lastSeenInBox[i] / 3;
			int boxCol = 3 * (num % 3) + lastSeenInBox[i] % 3;
			insertMove(board, i + 1, boxRow, boxCol, hints, moves);
			insertSudokuSet(seen, board);
		}
	}	
	return true;
}

void insertMove(Sudoku *board, char move, int row, int col, bool *hints, Stack *moves) {
	int boxTop = row - (row % 3);
	int boxLeft = col - (col % 3);
	for (int i = 0; i < 9; i++) {
		hints[81 * row + 9 * i + move - 1] = false;
		hints[81 * i + 9 * col + move - 1] = false;
		hints[81 * (boxTop + i / 3) + 9 * (boxLeft + i % 3) + move - 1] = false;
		hints[81 * row + 9 * col + i] = false;
	}
	board->b[row][col] = move;
	Move *val = malloc(sizeof(Move));
	Move tempval = {.i = row, .j = col, .val = move };
	*val = tempval;
	push(moves, val);
	/*printSudoku(board);*/
}

uint64_t ipow(int base, int exp) {
	int result = 1;
	for (int i = 0; i < exp; i++) {
		result *= base;
	}
	return result;
}

uint64_t sudokuHash(Sudoku *board) {
	uint64_t result = 0;
	int change = 5;
	int index = 8;
	int numzeros = 0;
	for (int i = 0; i < 81; i++) {
		int temp = board->b[index / 9][index % 9];
		numzeros += !temp;
		temp = (6 + temp) % 16;
		temp <<= (i * index * numzeros) % 60;
		result += temp; 
		index += change;
		index %= 81;
	}
	return result;
}

bool inSudokuSet(SudokuSet *boardSet, Sudoku *board) {
	uint64_t boardHash = sudokuHash(board);
	SudokuBucket *bucket = boardSet->set[boardHash % boardSet->buckets];
	while (bucket) {
		if (sudokuEquals(&bucket->board, board)) {
			return true;
		}
		bucket = bucket->next;
	}
	return false;
}

bool sudokuEquals(Sudoku *a, Sudoku *b) {
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (a->b[i][j] != b->b[i][j]) { return false; }
		}
	}
	return true;
}

bool insertSudokuSet(SudokuSet *boardSet, Sudoku *board) {
	if (boardSet->buckets == boardSet->stored) {
		resizeSudokuSet(boardSet);
	}
	uint64_t hash = sudokuHash(board);
	SudokuBucket *bucket = boardSet->set[hash % boardSet->buckets];
	if (bucket) {
		bool notIn = insertIntoBucket(bucket, board);
		if (notIn) {
			boardSet->stored++;
		}
		return notIn;
	}
	bucket = calloc(1, sizeof(SudokuBucket));
	bucket->board = *board;
	boardSet->set[hash % boardSet->buckets] = bucket;
	boardSet->stored++;
	return true;
}

void resizeSudokuSet(SudokuSet *boardSet) {
	SudokuBucket **newSet = calloc(boardSet->buckets * 2, sizeof(SudokuBucket *));
	for (int i = 0; i < boardSet->buckets; i++) {
		SudokuBucket *oldBucket = boardSet->set[i];
		while(oldBucket) {
			uint64_t hash = sudokuHash(&oldBucket->board);
			SudokuBucket *newBucket = newSet[hash % (boardSet->buckets * 2)];
			if (!newBucket) {
				newBucket = calloc(1, sizeof(SudokuBucket));
				newBucket->board = oldBucket->board;
				newSet[hash % (boardSet->buckets * 2)] = newBucket;
			} 
			else {
				insertIntoBucket(newBucket, &oldBucket->board);
			}
			SudokuBucket *temp = oldBucket;
			oldBucket = oldBucket->next;
			free(temp);
		}
	}	
	free(boardSet->set);
	boardSet->set = newSet;
	boardSet->buckets *= 2;
}

bool insertIntoBucket(SudokuBucket *bucket, Sudoku *board) {
	while(bucket->next) {
		if (sudokuEquals(&(bucket->board), board)) {
			/*printf("In\n");*/
			return false;
		}
		bucket = bucket->next;
	}
	if (sudokuEquals(&bucket->board, board)) { return false; }
	bucket->next = calloc(1, sizeof(SudokuBucket));
	bucket = bucket->next;
	bucket->board = *board;
	return true;
}

SudokuSet *initSet(int numBuckets) {
	SudokuSet *set = malloc(sizeof(SudokuSet));
	set->set = calloc(numBuckets, sizeof(SudokuBucket *));
	set->buckets = numBuckets;
	set->stored = 0;
	return set;
}

void deleteSet(SudokuSet *set) {
	int maxdepth = 0;
	for (int i = 0; i < set->buckets; i++) {
		int temp = deleteBucket(set->set[i]);
		if (temp > maxdepth) { maxdepth = temp; }
	}
	free(set->set);
	free(set);
}

int deleteBucket(SudokuBucket *bucket) {
	int temp = 0;
	while(bucket) {
		temp++;
		SudokuBucket *temp = bucket;
		bucket = bucket->next;
		free(temp);
	}
	/*printf("Bucket Length: %i\n", temp);*/
	return temp;
}

bool solvedBoard(Sudoku *board) {
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (!board->b[i][j]) { return false; }
		}
	}
	return true;
}

Stack *possibleMoves(Sudoku *board) {
	Stack *moves = initStack();
	bool *hints = getHints(board);
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (!board->b[i][j]) {
				for (int k = 0; k < 9; k++) {
					if (hints[81 * i + 9 * j + k]) {
						Move *m = malloc(sizeof(Move));
						m->i = i;
						m->j = j;
						m->val = k + 1;
						push(moves, m);
					}
				}	
			}
		}
	}
	free(hints);
	return moves;
}
