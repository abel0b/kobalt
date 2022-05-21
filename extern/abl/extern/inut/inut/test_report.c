#include <stdlib.h>
#include "inut/test.h"
#include "inut/test_report.h"
#include "inut/hash_table.h"
#include <stdio.h>

#define BOLD "\033[1m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define WHITE_FG_GREEN "\033[1;39;42m"
#define WHITE_FG_RED "\033[1;39;41m"
#define WHITE_FG_BLUE "\033[1;39;44m"
#define WHITE_FG_YELLOW "\033[1;39;43m"
#define RESET "\033[0m"
#define UNDERLINE "\033[4m"
#define TAB "       "

int number_total = 0;
int number_passed = 0;
int number_failed = 0;
int number_skipped = 0;
int number_todo = 0;

void
test_report_suite(char * complete_name) {
    printf (BOLD INUT_TEST_PREFIX "%s\n" RESET, complete_name);
}

void
test_report_case(struct TestCase * test_case, struct TestResult result) {
    number_total ++;
    switch(result.type) {
        case TEST_RESULT_PASS:
            printf (WHITE_FG_GREEN " PASS " RESET);
            number_passed ++;
            break;
        case TEST_RESULT_FAIL:
            printf (WHITE_FG_RED " FAIL " RESET);
            number_failed ++;
            break;
        case TEST_RESULT_TODO:
            printf (WHITE_FG_BLUE " TODO " RESET);
            number_todo ++;
            break;
        case TEST_RESULT_SKIP:
            printf (WHITE_FG_YELLOW " SKIP " RESET);
            number_todo ++;
            break;
        default:
            break;
    }
    printf(" %s\n", test_case->description);
    if (result.message != NULL) {
        printf("\n%s\n\n", result.message);
    }
}

void
test_report_progress_bar() {
    if (number_passed == number_total) return;
    int progress_pass = (float)number_passed / (float)number_total * 26;
    if (number_passed && progress_pass == 0) {
        progress_pass = 1;
    }

    int progress_fail = (float)number_failed / (float)number_total * 26;
    if (number_failed && progress_fail == 0) {
        progress_fail = 1;
    }

    int progress_todo = (float)number_todo / (float)number_total * 26;
    if (number_todo && progress_todo == 0) {
        progress_todo = 1;
    }

    int progress_skip = (float)number_skipped/ (float)number_total * 26;
    if (number_skipped && progress_skip == 0) {
        progress_skip = 1;
    }

    for(int i = 0; i < progress_pass; i++) {
        printf(GREEN "▄" RESET);
    }
    for(int i = 0; i < progress_fail; i++) {
        printf(RED "▄" RESET);
    }
    for(int i = 0; i < progress_todo; i++) {
        printf(BLUE "▄" RESET);
    }
    for(int i = 0; i < progress_skip; i++) {
        printf(YELLOW "▄" RESET);
    }
    printf("\n");
}

void
test_report_results(long seed, long duration) {
    printf("• done in %ldµs •\n\n", duration);

    if (number_passed) {
        printf (GREEN "%d of %d tests passed" RESET "\n", number_passed, number_total);
    }
    if (number_failed) {
        printf (RED "%d of %d tests failed" RESET "\n", number_failed, number_total);
    }
    if (number_todo) {
        printf (BLUE "%d of %d tests todo" RESET "\n", number_todo, number_total);
    }
    if (number_skipped) {
        printf (YELLOW "%d of %d tests skipped" RESET "\n", number_skipped, number_total);
    }

    test_report_progress_bar();

    printf("\nrandomized with seed " BOLD "%ld" RESET "\n", seed);
}
