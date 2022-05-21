#include "inut/inut.h"
#include "vec/vec.h"
#include "dict/dict.h"

int main(int argc, char * argv[]) {
    suite vec = test_suite(NULL, "vec");
    test_case(vec, "vec", test_vec);
    suite dict = test_suite(NULL, "dict");
    test_case(dict, "dict", test_dict);

    return test_run(argc, argv);
}
