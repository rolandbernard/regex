#ifndef _TEST_H_
#define _TEST_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int failed;
    int passed;
    const char* msg; 
    int exm;
    int mat;
} TestResult;
const TestResult SUCCESS = { 0, 1, "", -1, -1 };
typedef TestResult (*TestFunction)();
typedef struct {
    const char* name;
    TestFunction function;
} Test;

#define LEN(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))
#define ASSERT(COND) { if (!(COND)) { TestResult ret = { 1, 0, "Failed assertion: "  #COND, -1, -1 }; return ret; }; }
#define ASSERT_EX(COND, EX) { if (!(COND)) { TestResult ret = { 1, 0, "Failed assertion: "  #COND, EX, -1 }; return ret; }; }
#define ASSERT_EX_MA(COND, EX, MA) { if (!(COND)) { TestResult ret = { 1, 0, "Failed assertion: "  #COND, EX, MA }; return ret; }; }
#define TEST(NAME) { .name = #NAME, .function = NAME }

#endif