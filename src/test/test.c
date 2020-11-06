
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
    const char* msg; 
} TestResult;
const TestResult SUCCESS = { NULL };
typedef TestResult (*TestFunction)();
typedef struct {
    const char* name;
    TestFunction function;
} Test;

#define TEST(function) { #function, function }

TestResult an_empty_test_sould_succeed() {
    return SUCCESS;
}

Test tests[] = {
    TEST(an_empty_test_sould_succeed)
};

int main() {
    int num_fails = 0;
    int num_pass = 0;
    fprintf(stderr, "\n");
    for(int t = 0; t < sizeof(tests) / sizeof(tests[0]); t++) {
        TestResult result = { NULL };
        if(tests[t].function != NULL) {
            result = tests[t].function();
        }
        if(result.msg == NULL) {
            fprintf(stderr, "\e[32mPassed\e[m test '%s'\n", tests[0].name);
            num_pass++;
        } else {
            fprintf(stderr, "\e[31mFailed\e[m test '%s'\n", tests[0].name);
            num_fails++;
        }
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "\e[32mPassed %i tests\e[m\n", num_pass);
    fprintf(stderr, "\e[31mFailed %i tests\e[m\n", num_fails);
    return num_fails;
}
