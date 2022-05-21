#include "abl/dict.h"
#include "dict.h"

test test_dict(void) {
    struct abl_dict mydict;
    abl_dict_new(&mydict);
    for(int i = 0; i < mydict.numbuckets; ++i) {
    	assert_int_zero(mydict.sizes[i]);
    }

    int a = 42;
    int b = 1337;
    int c = 1024;

    abl_dict_set(&mydict, "foo", &a);
    assert_ptr_equal(&a, abl_dict_get(&mydict, "foo"));

    abl_dict_set(&mydict, "foo", &b);
    assert_ptr_equal(&b, abl_dict_get(&mydict, "foo"));

    abl_dict_set(&mydict, "", &c);
    assert_ptr_equal(&c, abl_dict_get(&mydict, ""));

    assert_ptr_equal(&b, abl_dict_get(&mydict, "foo"));

    abl_dict_del(&mydict);

    return pass();
}
