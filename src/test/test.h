#ifndef _TEST_H_
#define _TEST_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int failed;
    int passed;
    const char* msg; 
} TestResult;
const TestResult SUCCESS = { 0, 1, "" };
typedef TestResult (*TestFunction)(int);
typedef struct {
    const char* name;
    TestFunction function;
} Test;

#define ASSERT(COND) { if (!(COND)) { TestResult ret = { 1, 0, "Failed assertion: "  #COND }; return ret; }; }

#define TEST(NAME, BODY) { #NAME, ({ TestResult NAME(int dep) { BODY; return SUCCESS; }; NAME; }) }
#define TEST_SET(NAME, ...) { #NAME, ({ \
    TestResult NAME(int dep) { \
        char ind[dep * 2 + 2 + 1]; \
        for (int i = 0; i < dep * 2 + 2; i++) { ind[i] = ' '; } \
        ind[dep * 2] = 0; \
        TestResult ret = { 0, 0, NULL }; \
        fprintf(stderr, "%s" #NAME ":\n", ind); \
        ind[dep * 2] = ' '; ind[dep * 2 + 2] = 0; \
        Test tests [] = { __VA_ARGS__ }; \
        for(int t = 0; t < sizeof(tests) / sizeof(tests[0]); t++) { \
            TestResult result = tests[t].function(dep + 1); \
            if (result.msg != NULL) { \
                if(result.failed == 0) {  \
                    fprintf(stderr, "%s\e[32mPassed\e[m test '%s'\n", ind, tests[0].name); \
                } else { \
                    fprintf(stderr, "%s\e[31mFailed\e[m test '%s'\n%s  \e[31m|\e[m %s\n", ind, tests[0].name, ind, result.msg); \
                } \
            } \
            ret.passed += result.passed; \
            ret.failed += result.failed; \
        } \
        return ret; \
    }; \
    NAME; \
}) }
#define RUN_TESTS(...) ({ \
    int num_fails = 0; \
    int num_pass = 0; \
    fprintf(stderr, "\n"); \
    Test tests [] = { __VA_ARGS__ }; \
    for(int t = 0; t < sizeof(tests) / sizeof(tests[0]); t++) { \
        TestResult result = tests[t].function(0); \
        if (result.msg != NULL) { \
            if(result.failed == 0) {  \
                fprintf(stderr, "\e[32mPassed\e[m test '%s'\n", tests[0].name); \
            } else { \
                fprintf(stderr, "\e[31mFailed\e[m test '%s'\n  \e[31m|\e[m %s\n", tests[0].name, result.msg); \
            } \
        } \
        num_pass += result.passed; \
        num_fails += result.failed; \
    } \
    fprintf(stderr, "\n"); \
    fprintf(stderr, "\e[32mPassed %i tests\e[m\n", num_pass); \
    fprintf(stderr, "\e[31mFailed %i tests\e[m\n", num_fails); \
    num_fails; \
})

#endif