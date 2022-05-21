#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "inut/array.h"
#include "inut/test.h"
#include "inut/test_report.h"
#include "inut/assert.h"

char _inut_buffer[INUT_MAX_STR];

#if WINDOWS
#include <windows.h>
// credit: https://stackoverflow.com/a/26085827
int gettimeofday(struct timeval * tp, struct timezone * tzp) {
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#else
#include <sys/time.h>
#endif

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
    root->name = INUT_TEST_ROOT;
    strcpy(root->complete_name, "\0");
    root->test_suites = array_make();
    root->test_cases = array_make();
}

struct TestSuite *
test_suite(struct TestSuite * suite, char * name) {
    if (root == NULL) {
        test_init();
    }

    struct TestSuite * parent_suite = (suite == NULL)? root : suite;

    struct TestSuite * new_suite = malloc (sizeof (struct TestSuite));
    new_suite->parent = parent_suite;
    new_suite->name = name;
    strcpy(new_suite->complete_name, "\0");
    new_suite->test_suites = array_make();
    new_suite->test_cases = array_make();
    array_push(parent_suite->test_suites, new_suite);
    return new_suite;
}

struct TestCase *
test_case(struct TestSuite * suite, char * description, struct TestResult (*test)()) {
    struct TestSuite * parent_suite = (suite == NULL)? root : suite;

    struct TestCase * test_case = malloc (sizeof (struct TestCase));
    test_case->description = description;
    test_case->test = test;
    array_push (parent_suite->test_cases, test_case);
    return test_case;
}

int test_run(int argc, char * argv[]) {
    struct TestSuite * suite;
    struct timeval t1, t2;

    struct TestSuiteIterator * suite_it;
    long seed = 0;
    int opt;
    int seed_arg = 0;

    int i = 1;
    while(i<argc) {
        if (strcmp(argv[i], "-s") == 0) {
	    ++i;
	    seed_arg = 1;
            seed = atol(argv[i]);
	}
	else {
	    fprintf(stderr, "error: unknown option '%s'\n", argv[i]);
	    exit(1);
	}
	++i;
    }

    if (!seed_arg) {
        gettimeofday(&t1, NULL);
        seed =  t1.tv_sec * (int)1e6 + t1.tv_usec;
    }

    for(suite_it = test_suite_it_make (root);
        !test_suite_it_done (suite_it);
        test_suite_it_next (suite_it)) {
        suite = test_suite_it_get (suite_it);

        if (strcmp(suite->name, INUT_TEST_ROOT) != 0) {
            if (strcmp(suite->parent->name, INUT_TEST_ROOT) != 0) {
                strcat(suite->complete_name, suite->parent->complete_name);
                strcat(suite->complete_name, INUT_TEST_SEPARATOR);
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

    gettimeofday(&t2, NULL);
    long elapsed = (t2.tv_sec - t1.tv_sec) * 1e6 + (t2.tv_usec - t1.tv_usec);

    test_report_results(seed, elapsed);
    test_suite_destroy(root);
    
    if (number_failed) {
        return 1;
    }
    return 0;
}
