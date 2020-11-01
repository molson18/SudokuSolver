#include <stdlib.h>

typedef struct Stack {
	void **stack;
	int maxVals;
	int length;
} Stack;

void *pop(Stack *stack);
void push(Stack *stack, void *item);
void deleteStack(Stack *stack);
Stack *initStack();
