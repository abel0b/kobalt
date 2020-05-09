#ifndef TEST_ASSERT_H
#define TEST_ASSERT_H

#include "test/test_report.h"

#define MAX_STR 256

char buffer[MAX_STR];

#define assert_zero(a) assert_int_equal(a, 0)

#define assert_int_equal(a, b) if(a!=b) {\
    sprintf(buffer, TAB RED "• Error: Expected integer '%d' to equal '%d'" RESET "\n\n" TAB GREEN "+ expected" RESET " " RED "- actual" RESET "\n\n" TAB GREEN "+ %d" RESET "\n" TAB RED "- %d" RESET "\n\n" TAB "at %s, line %d", a, b, a, b, __FILE__, __LINE__);\
    return fail(buffer);\
}

#define assert_ptr_equal(a, b) if(a!=b) {\
    char ptr1[64];\
    char ptr2[64];\
    if (a == NULL) {\
        sprintf(ptr1, "NULL");\
    }\
    else {\
        sprintf(ptr1, "%p", a);\
    }\
    if (b == NULL) {\
        sprintf(ptr2, "NULL");\
    }\
    else {\
        sprintf(ptr2, "%p", b);\
    }\
    sprintf(buffer, TAB RED "• Error: Expected pointer '%s' to equal '%s'" RESET "\n\n" TAB GREEN "+ expected" RESET " " RED "- actual" RESET "\n\n" TAB GREEN "+ %s" RESET "\n" TAB RED "- %s" RESET "\n\n" TAB "at %s, line %d", ptr1, ptr2, ptr1, ptr2, __FILE__, __LINE__);\
    return fail(buffer);\
}

#define assert(a) assert_boolean(a)

#define assert_boolean(a) if(!a) {\
    sprintf(buffer, TAB RED "• Error: Expected 'false' to be 'true'" RESET "\n\n" TAB GREEN "+ expected" RESET " " RED "- actual" RESET "\n\n" TAB GREEN "+ true" RESET "\n" TAB RED "- false" RESET "\n\n" TAB "at %s, line %d", __FILE__, __LINE__);\
    return fail(buffer);\
}

#endif
