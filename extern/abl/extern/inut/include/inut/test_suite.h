#ifndef INUT_TEST_SUITE_H
#define INUT_TEST_SUITE_H

#include "inut/array.h"

struct TestSuite {
    struct TestSuite * parent;
    char * name;
    char complete_name[256];
    struct Array * test_suites;
    struct Array * test_cases;
};

struct TestSuiteIterator {
    struct Stack * stack;
    struct TestSuite * begin;
    struct TestSuite * current;
};

enum TestResultType {
    TEST_RESULT_PASS,
    TEST_RESULT_FAIL,
    TEST_RESULT_SKIP,
    TEST_RESULT_TODO
};

struct TestResult {
    enum TestResultType type;
    char * message;
};

struct TestCase {
    char * description;
    struct TestResult (*test)();
};

typedef struct TestResult test;

typedef struct TestSuite * suite;

struct TestSuite * test_suite_resolve(struct TestSuite * suite, char * name);

void test_case_destroy(struct TestCase * test);

struct TestSuiteIterator * test_suite_it_make (struct TestSuite * suite);

void test_suite_it_next (struct TestSuiteIterator * suite_it);

int test_suite_it_done (struct TestSuiteIterator * suite_it);

void test_suite_it_destroy(struct TestSuiteIterator * suite);

struct TestSuite * test_suite_it_get (struct TestSuiteIterator * suite_it);

void test_suite_destroy(struct TestSuite * suite);

#endif
