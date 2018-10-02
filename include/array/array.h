#ifndef ARRAY_H
#define ARRAY_H

struct Array {
    void * * data;
    int size;
    int capacity;
};

struct ArrayIterator {
    struct Array * array;
    int position;
};

struct Stack {
    struct StackItem * head;
    int size;
};

struct StackItem {
    void * value;
    struct StackItem * next;
};

int
array_is_empty(struct Array * array);

struct Array * array_make();

int array_push(struct Array * array, void * value);

void array_destroy(struct Array * array);

struct Stack * stack_make();

int stack_is_empty(struct Stack * stack);

void stack_push(struct Stack * stack, void * value);

void * stack_pop(struct Stack * stack);

void stack_destroy(struct Stack * stack);

struct ArrayIterator array_it_make(struct Array * array);

struct ArrayIterator array_it_next(struct ArrayIterator iterator);

void * array_it_get(struct ArrayIterator iterator);

int array_it_end(struct ArrayIterator iterator);

#endif
