#ifndef TEST_REPORT_H
#define TEST_REPORT_H

#include "test/test_suite.h"

#define BOLD "\033[1m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define WHITE_FG_GREEN "\033[1;30;42m"
#define WHITE_FG_RED "\033[1;37;41m"
#define WHITE_FG_BLUE "\033[1;37;44m"
#define WHITE_FG_YELLOW "\033[1;37;43m"
#define RESET "\033[0m"
#define TEST_PREFIX "------ "
#define TEST_SUFFIX ""
#define TEST_SEPARATOR " > "
#define UNDERLINE "\033[4m"
#define TAB "       "

void
test_report_suite(char * complete_name);

void
test_report_case(struct TestCase * test_case, struct TestResult result);

void
test_report_results(long seed, long duration);


#endif
