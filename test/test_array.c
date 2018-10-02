#include <stdio.h>
#include "test_array.h"
#include "array/array.h"
#include "test/assert.h"

typedef struct TestResult test;

test test_initial_capacity () {
    struct Array * array = array_make ();
    assert_int_equal (array->capacity, 2);
    array_destroy (array);
    return pass ();
}

test test_initial_size () {
    struct Array * array = array_make ();
    assert_zero (array->size);
    array_destroy (array);
    return pass ();
}

test test_empty_array () {
    struct Array * array = array_make ();
    assert (array_is_empty (array));
    array_destroy (array);
    return pass ();
}

test test_array_push () {
    int foo, bar = 42;
    struct Array * array = array_make ();
    array_push (array, &foo);
    assert_int_equal(array->size, 1);
    assert_ptr_equal(array->data[0], &foo);
    array_push (array, &bar);
    assert_int_equal(array->size, 2);
    assert_ptr_equal(array->data[0], &foo);
    assert_ptr_equal(array->data[1], &bar);
    array_destroy (array);
    return pass ();
}

void
test_array() {
    test_suite(TEST_ROOT, "array");
    test_case("array", "array initial capacity is 2", test_initial_capacity);
    test_case("array", "array initial size is 0", test_initial_size);
    test_case("array", "initial array is empty", test_empty_array);
    test_case("array", "push values to array", test_array_push);
}
