#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test/test.h"
#include "array/array.h"
#include "test/test_report.h"
#include <sys/time.h>

struct TestSuite * root = NULL;

struct Array * tests = NULL;
int id_count = 0;

struct TestResult
pass () {
    return (struct TestResult) {TEST_RESULT_PASS, NULL};
}

struct TestResult
fail (char * diagnostic) {
    return (struct TestResult) {TEST_RESULT_FAIL, diagnostic};
}

struct TestResult
skip () {
    return (struct TestResult) {TEST_RESULT_SKIP, NULL};
}

struct TestResult
todo () {
    return (struct TestResult) {TEST_RESULT_TODO, NULL};
}

void
test_init () {
    root = malloc (sizeof (struct TestSuite));
    root->parent = root;
    root->name = TEST_ROOT;
    strcpy(root->complete_name, "\0");
    root->test_suites = array_make();
    root->test_cases = array_make();
}

void
test_suite(char * parent, char * name) {
    if (root == NULL) {
        test_init();
    }
    struct TestSuite * parent_suite = test_suite_resolve(root, parent);

    if(parent_suite == NULL) {
        printf("test suite not found\n");
        exit(1);
    }

    struct TestSuite * new_suite = malloc (sizeof (struct TestSuite));
    new_suite->parent = parent_suite;
    new_suite->name = name;
    strcpy(new_suite->complete_name, "\0");
    new_suite->test_suites = array_make();
    new_suite->test_cases = array_make();
    array_push(parent_suite->test_suites, new_suite);
}

void test_case(char * suite, char * description, struct TestResult (*test)()) {
    struct TestSuite * parent_suite = test_suite_resolve(root, suite);

    if(parent_suite == NULL) {
        printf("test suite not found\n");
        exit(1);
    }

    struct TestCase * test_case = malloc (sizeof (struct TestCase));
    test_case->description = description;
    test_case->test = test;
    array_push (parent_suite->test_cases, test_case);
}

int
test_run () {
    struct TestSuite * suite;
    struct timeval currentTime;

    struct TestSuiteIterator * suite_it;
    long seed;

    gettimeofday(&currentTime, NULL);
    seed =  currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;

    for(suite_it = test_suite_it_make (root);
        !test_suite_it_done (suite_it);
        test_suite_it_next (suite_it)) {
        suite = test_suite_it_get (suite_it);

        if (strcmp(suite->name, TEST_ROOT) != 0) {
            if (strcmp(suite->parent->name, TEST_ROOT) != 0) {
                strcat(suite->complete_name, suite->parent->complete_name);
                strcat(suite->complete_name, TEST_SEPARATOR);
            }

            strcat(suite->complete_name, suite->name);

            if (!array_is_empty (suite->test_cases)) {
                test_report_suite(suite->complete_name);
            }
        }

        if (!array_is_empty(suite->test_cases)) {
            struct ArrayIterator it = array_it_make(suite->test_cases);
            struct TestCase * test;
            while(!array_it_end(it)) {
                test = array_it_get(it);
                test_report_case(test, test->test());
                it = array_it_next(it);
            }
            printf("\n");
        }
    }

    test_suite_it_destroy(suite_it);

    gettimeofday(&currentTime, NULL);
    long duration = currentTime.tv_sec * (int)1e6 + currentTime.tv_usec - seed;

    test_report_results(seed, duration);
    test_suite_destroy(root);
    return 0;
}
