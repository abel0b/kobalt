#ifndef INUT_TEST_REPORT_H
#define INUT_TEST_REPORT_H

#include "inut/test_suite.h"

extern int number_total;
extern int number_passed;
extern int number_failed;
extern int number_skipped;
extern int number_todo;

void
test_report_suite(char * complete_name);

void
test_report_case(struct TestCase * test_case, struct TestResult result);

void
test_report_results(long seed, long duration);


#endif
