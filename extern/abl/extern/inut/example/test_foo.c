#include "inut/inut.h"

int sum(int a, int b) {
    return a+b;
}

test test_sum() {
    assert_int_equal(sum(1,2), 3);
    assert_int_equal(sum(4,2), 6);
    assert_int_equal(sum(-5,2), -3);
    return pass();
}

test test_fail() {
	assert_int_equal(0,1);
	return pass();
}

int main(int argc, char * argv[]) {
    suite math = test_suite(NULL, "math");
    test_case(math, "sum works", test_sum);
    test_case(math, "fail test", test_fail);

    return test_run(argc, argv);
}