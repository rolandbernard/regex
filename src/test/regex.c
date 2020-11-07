// These tests only compile using the gcc ententions nested functions and expression statements

#include "regex.h"
#include "test/test.h"

int main() {
    return RUN_TESTS(
        TEST(compiling_a_valid_regex_returns_non_null, ({
            const char* examples[] = { "test", "ab*c", "d(e?f)*", "(qe)?*d+" };
            for(int e = 0; e < LEN(examples); e++) {
                Regex regex = compileMatchingRegex(examples[e]);
                ASSERT(regex != NULL);
                disposeRegex(regex);
            }
        })),
        TEST(compiling_a_invalid_regex_returns_null, ({
            const char* examples[] = { "test[", "test{", "**", "++", "??", "]" };
            for(int e = 0; e < LEN(examples); e++) {
                Regex regex = compileMatchingRegex(examples[e]);
                ASSERT(regex == NULL);
                disposeRegex(regex);
            }
        })),
        TEST(simple_strings_match_themselfs, ({
            const char* examples[] = { "test", "abc", "def", "qed" };
            for(int e = 0; e < LEN(examples); e++) {
                Regex regex = compileMatchingRegex(examples[e]);
                ASSERT(matchRegex(regex, examples[e], NULL));
                disposeRegex(regex);
            }
        })),
        TEST(simple_strings_dont_match_other_strings, ({
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
        }))
    );
}
