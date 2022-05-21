#include "abl/vec.h"
#include "vec.h"

test test_vec(void) {
    struct abl_vec myvec;
    abl_vec_new(&myvec, sizeof(int));
    assert(abl_vec_empty(&myvec));
    assert_int_equal(0, myvec.size);
    assert_uint_equal(sizeof(0), myvec.elem_size);
    int val = 42;
    abl_vec_push(&myvec, &val);
    assert_int_equal(1, myvec.size);
    val = 1024;
    abl_vec_push(&myvec, &val);
    int val2;
    abl_vec_peek(&myvec, &val2);
    assert_int_equal(1024, val2);
    assert_int_equal(2, myvec.size);
    assert_int_equal(42, *(int *)abl_vec_get(&myvec, 0));
    assert_int_equal(1024, *(int *)abl_vec_get(&myvec, 1));
    assert_int_equal(2, myvec.size);
    abl_vec_pop(&myvec, &val);
    assert_int_equal(1024, val);
    abl_vec_pop(&myvec, &val);
    assert_int_equal(42, val);
    assert_int_equal(0, myvec.size);
    abl_vec_del(&myvec);
    return pass();
}
