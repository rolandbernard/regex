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
typedef TestResult (*TestFunction)();
typedef struct {
    const char* name;
    TestFunction function;
} Test;

#define LEN(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))
#define ASSERT(COND) { if (!(COND)) { TestResult ret = { 1, 0, "Failed assertion: "  #COND }; return ret; }; }
#define TEST(NAME) { .name = #NAME, .function = NAME }

#endif