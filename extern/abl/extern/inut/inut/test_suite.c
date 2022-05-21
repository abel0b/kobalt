#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inut/test_suite.h"
#include "inut/array.h"

struct TestSuite *
test_suite_resolve(struct TestSuite * suite, char * name) {
    struct Stack * stack = stack_make();
    stack_push(stack, suite);
    while(!stack_is_empty(stack)) {
        suite = stack_pop(stack);

        if (strcmp(suite->name, name) == 0) {
            stack_destroy(stack);
            return suite;
        }
        struct ArrayIterator it = array_it_make(suite->test_suites);
        while(!array_it_end(it)) {
            stack_push(stack, array_it_get(it));
            it = array_it_next(it);
        }
    }
    stack_destroy(stack);
    return NULL;
}

struct TestSuiteIterator *
test_suite_it_make (struct TestSuite * suite) {
    struct TestSuiteIterator * suite_it = malloc (sizeof (struct TestSuiteIterator));

    suite_it->stack = stack_make ();
    suite_it->current = suite;

    return suite_it;
}

void
test_suite_it_next (struct TestSuiteIterator * suite_it) {
    struct ArrayIterator it = array_it_make (suite_it->current->test_suites);
    while(!array_it_end(it)) {
        stack_push(suite_it->stack, array_it_get(it));
        it = array_it_next(it);
    }
    if (stack_is_empty (suite_it->stack)) {
        suite_it->current = NULL;
    }
    else {
        suite_it->current = stack_pop (suite_it->stack);
    }
}

int
test_suite_it_done (struct TestSuiteIterator * suite_it) {
    return suite_it->current == NULL;
}

struct TestSuite *
test_suite_it_get (struct TestSuiteIterator * suite_it) {
    return suite_it->current;
}

void
test_suite_it_destroy (struct TestSuiteIterator * suite_it) {
    stack_destroy (suite_it->stack);
    free (suite_it);
}

void
test_suite_destroy(struct TestSuite * suite) {
    struct TestSuiteIterator * suite_it = test_suite_it_make (suite);
    while (!test_suite_it_done (suite_it)) {
        suite = test_suite_it_get (suite_it);
        test_suite_it_next(suite_it);
        if (!array_is_empty(suite->test_cases)) {
            struct ArrayIterator it = array_it_make(suite->test_cases);
            struct TestCase * test;
            while(!array_it_end(it)) {
                test = array_it_get(it);
                test_case_destroy (test);
                it = array_it_next(it);
            }
        }
        array_destroy(suite->test_suites);
        array_destroy(suite->test_cases);
        free (suite);
    }
    test_suite_it_destroy(suite_it);
}

void test_case_destroy(struct TestCase * test) {
    free (test);
}
