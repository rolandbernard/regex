
#include "regex.h"
#include "test/test.h"
        
TestResult compiling_a_valid_regex_returns_non_null() {
    const char* examples[] = { "(qe)?*d+", "test", "ab*c", "d(e?f)*", "(qe)?*d+", "(a?b+){3}", "(a?b+c){1,4}", "(x?yz){3,}" };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        ASSERT(regex != NULL);
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult compiling_a_invalid_regex_returns_null() {
    const char* examples[] = { "test[", "test{", "**", "++", "??", "{", "{5}", "ab(c" };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        ASSERT(regex == NULL);
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult simple_strings_match_themselfs() {
    const char* examples[] = { "test", "abc", "def", "qed" };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        ASSERT(matchRegex(regex, examples[e], NULL));
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult simple_strings_dont_match_other_strings() {
    const char* examples[] = { "test", "abc", "def", "qed" };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; m < LEN(examples); m++) {
            if(m != e) {
                ASSERT(!matchRegex(regex, examples[m], NULL));
            }
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

static Test tests[] = {
    TEST(compiling_a_valid_regex_returns_non_null),
    TEST(compiling_a_invalid_regex_returns_null),
    TEST(simple_strings_match_themselfs),
    TEST(simple_strings_dont_match_other_strings),
};

int main() {
    int num_fails = 0;
    int num_pass = 0;
    fprintf(stderr, "\n");
    for(int t = 0; t < LEN(tests); t++) {
        TestResult result = tests[t].function();
        if (result.msg != NULL) {
            if(result.failed == 0) {
                fprintf(stderr, "\e[32mPassed\e[m test '%s'\n", tests[t].name);
            } else {
                fprintf(stderr, "\e[31mFailed\e[m test '%s'\n  \e[31m|\e[m %s\n", tests[t].name, result.msg);
            }
        }
        num_pass += result.passed;
        num_fails += result.failed;
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "\e[32mPassed %i tests\e[m\n", num_pass);
    fprintf(stderr, "\e[31mFailed %i tests\e[m\n", num_fails);
    return num_fails;
}
