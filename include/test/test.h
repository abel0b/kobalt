#ifndef TEST_H
#define TEST_H

#include "test/test_suite.h"

#define TEST_ROOT "root"

void test_suite(char * parent, char * name);

int test_case_id_gen();

void test_case(char * parent, char * name, struct TestResult (*test)());

struct TestResult pass();

struct TestResult fail(char * diagnostic);

struct TestResult skip();

struct TestResult todo();

int test_run();

#endif
