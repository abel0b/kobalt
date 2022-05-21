#include "inut/array.h"
#include <stdlib.h>

struct Array *
array_make () {
    struct Array * array = malloc (sizeof (struct Array));
    array->size = 0;
    array->capacity = 2;
    array->data = malloc (array->capacity * sizeof (void *));
    return array;
}

void *
array_last(struct Array * array) {
    return array->data[array->size-1];
}

int
array_length(struct Array * array) {
    return array->size;
}

int
array_push (struct Array * array, void * value) {
    if (array->size == array->capacity) {
        array->capacity = 2 * array->capacity;
        array->data = realloc (array->data, sizeof (void *) * array->capacity);
    }
    array->data[array->size] = value;
    array->size ++;
    return 1;
}

int
array_is_empty(struct Array * array) {
    return array->size == 0;
}

void
array_destroy (struct Array * array) {
    free (array->data);
    free (array);
}

struct Stack *
stack_make () {
    struct Stack * stack = malloc (sizeof (struct Stack));
    stack->size = 0;

    return stack;
}

int stack_is_empty(struct Stack * stack) {
    return stack->size == 0;
}

void
stack_push (struct Stack * stack, void * value) {
    struct StackItem * item = malloc (sizeof (struct StackItem));
    item->value = value;
    item->next = NULL;

    if (stack->size == 0) {
        stack->head = item;
    }
    else {
        struct StackItem * it = stack->head;
        while(it->next != NULL) {
            it = it->next;
        }
        it->next = item;
    }

    stack->size ++;
}

void *
stack_pop (struct Stack * stack) {
    struct StackItem * head = stack->head;
    void * value = stack->head->value;
    if (stack->size > 1) {
        stack->head = stack->head->next;
    }
    free (head);
    stack->size --;
    return value;
}

void
stack_destroy(struct Stack * stack) {
    if (stack->size > 0) {
        struct StackItem * it = stack->head;
        while(it->next != NULL) {
            struct StackItem * tmp = it;
            it = it->next;
            free (tmp);
        }
        free (it);
    }
    free (stack);
}

struct ArrayIterator
array_it_make(struct Array * array) {
    struct ArrayIterator iterator;
    iterator.position = 0;
    iterator.array = array;
    return iterator;
}

struct ArrayIterator
array_it_next(struct ArrayIterator iterator) {
    iterator.position ++;
    return iterator;
}

void *
array_it_get(struct ArrayIterator iterator) {
    return iterator.array->data[iterator.position];
}

int
array_it_end(struct ArrayIterator iterator) {
    return iterator.position == iterator.array->size;
}
