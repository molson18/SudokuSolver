#include "Stack.h"

void *pop(Stack *stack) {
	if (stack->length == 0) {
		return NULL;
	}
	stack->length--;
	return stack->stack[stack->length];
}

void push(Stack *stack, void *item) {
	if (stack->length >= stack->maxVals) {
		stack->stack = realloc(stack->stack, sizeof(void *) * stack->maxVals * 2);
		stack->maxVals *= 2;
	}
	stack->stack[stack->length] = item;
	stack->length++;
	return;
}

void deleteStack(Stack *stack) {
	for (int i = 0; i < stack->length; i++) {
		free(stack->stack[i]);
	}
	free(stack->stack);
	free(stack);
}

Stack *initStack() {
	Stack *stack = malloc(sizeof(Stack));
	stack->stack = malloc(sizeof(void *));
	stack->maxVals = 1;
	stack->length = 0;
	return stack;
}
