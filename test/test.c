
#include "regex.h"
#include "regex-type.h"
#include "test.h"
        
TestResult cheching_a_valid_regex_returns_negative_one() {
    const char* examples[] = {"test", "ab*c", "d(e?f)*", "(qe)?*d+", "(a?b+){3}", "(a?b+c){1,4}", "(x?yz){3,}", "((a+b)+c|cb|de|cd)+", "\\s" };
    for(int e = 0; e < LEN(examples); e++) {
        int error = getRegexErrorLocation(examples[e]);
        ASSERT_EX(error == -1, e);
    }
    return SUCCESS;
}

TestResult checking_a_invalid_regex_returns_the_error_location() {
    const char* examples[] = { "test[", "test{", "**", "++", "??", "{", "{5}", "ab(c", "a{0}", "a{0,0}" };
    int errors[] = { 4, 4, 0, 0, 0, 0, 0, 2, 1, 1 };
    for(int e = 0; e < LEN(examples); e++) {
        int error = getRegexErrorLocation(examples[e]);
        ASSERT_EX(error == errors[e], e);
    }
    return SUCCESS;
}
        
TestResult compiling_a_valid_regex_returns_non_null() {
    const char* examples[] = {"test", "ab*c", "d(e?f)*", "(qe)?*d+", "(a?b+){3}", "(a?b+c){1,4}", "(x?yz){3,}", "((a+b)+c|cb|de|cd)+" };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        ASSERT_EX(regex != NULL, e);
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult compiling_a_invalid_regex_returns_null() {
    const char* examples[] = { "test[", "test{", "**", "++", "??", "{", "{5}", "ab(c", "a{0}", "a{0,0}" };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        ASSERT_EX(regex == NULL, e);
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult simple_strings_match_themselfs() {
    const char* examples[] = { "test", "abc", "def", "qed" };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        ASSERT_EX(matchRegex(regex, examples[e], NULL), e);
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
                ASSERT_EX_MA(!matchRegex(regex, examples[m], NULL), e, m);
            }
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult using_the_pipe_matches_one_of_the_options() {
    const char* examples[] = { "a|b", "a|b|c|d|e", "a|bc|def|ghijk" };
    const char* example_match[][256] = {
        { "a", "b", NULL },
        { "a", "b", "c", "d", "e", NULL },
        { "a", "bc", "def", "ghijk", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult using_the_pipe_doesnt_match_a_string_not_in_the_options() {
    const char* examples[] = { "a|b", "a|b|c|d|e", "a|bc|def|ghijk" };
    const char* example_match[][256] = {
        { "ab", "c", NULL },
        { "ab", "ab", "cd", "df", "g", NULL },
        { "b", "c", "df", "ghij", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(!matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult using_the_star_matches_zero_or_more_of_the_previous_group() {
    const char* examples[] = { "a*", "(ab)*", "(abc)*" };
    const char* example_match[][256] = {
        { "", "aa", "aaa", "aaaa", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", NULL },
        { "", "abababab", "ab", "ababababab", "ababab", NULL },
        { "", "abc", "abcabc", "abcabcabcabcabc", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult using_the_plus_matches_one_or_more_of_the_previous_group() {
    const char* examples[] = { "a+", "(ab)+", "(abc)+" };
    const char* example_match[][256] = {
        { "a", "aa", "aaa", "aaaa", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", NULL },
        { "abababab", "ab", "ababababab", "ababab", NULL },
        { "abc", "abcabc", "abcabcabcabcabc", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult using_the_plus_doesnt_match_zero_repetitions() {
    const char* examples[] = { "a+", "(ab)+", "(abc)+" };
    const char* example_match[][256] = {
        { "", NULL },
        { "", NULL },
        { "", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(!matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult using_the_question_mark_matches_zero_or_one_of_the_previous_group() {
    const char* examples[] = { "a?", "(ab)?", "(abc)?" };
    const char* example_match[][256] = {
        { "", "a", NULL },
        { "", "ab", NULL },
        { "", "abc", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult using_the_question_mark_doesnt_match_more_than_one_of_the_previous_group() {
    const char* examples[] = { "a?", "(ab)?", "(abc)?" };
    const char* example_match[][256] = {
        { "aa", "aaaaa", "aaaaaaaa", "aaaaaaaaaaaa", NULL },
        { "abab", "ababab", "ababababab", "abababababababab", NULL },
        { "abcabc", "abcabcabc", "abcabcabcabcabc", "abcabcabcabcabcabcabc", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(!matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult using_the_curly_bacets_can_specify_the_repetitions_of_the_previous_group() {
    const char* examples[] = { "a{3}", "(ab){2}", "(abc){3}" };
    const char* example_match[][256] = {
        { "aaa", NULL },
        { "abab", NULL },
        { "abcabcabc", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult using_the_curly_bacets_can_specify_a_range_of_repetitions_of_the_previous_group() {
    const char* examples[] = { "a{2,5}", "(ab){2,}", "(abc){,3}", "(ef){0,3}", "g{0,}", "h{,}" };
    const char* example_match[][256] = {
        { "aa", "aaa", "aaaa", "aaaaa", NULL },
        { "abab", "abababab", "abababababababab", "ababababababababababab", NULL },
        { "", "abc", "abcabc", "abcabcabc", NULL },
        { "", "ef", "efef", "efefef", NULL },
        { "", "g", "gg", "gggggg", "gggggggggggg", "ggggggggggggggggg", NULL },
        { "", "h", "hh", "hhhhhh", "hhhhhhhhhhhh", "hhhhhhhhhhhhhhhhh", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult using_the_curly_bacets_doesnt_match_repetitions_outside_the_range() {
    const char* examples[] = { "a{2,5}", "(ab){2,}", "(abc){,3}", "(ef){0,3}", "a{3}", "(ab){2}", "(abc){3}" };
    const char* example_match[][256] = {
        { "", "a", "aaaaaa", "aaaaaaa", "aaaaaaaaa", NULL },
        { "", "ab", NULL },
        { "abcabcabcabc", "abcabcabcabcabcabc", "abcabcabcabcabcabcabcabc", NULL },
        { "efefefef", "efefefefef", "efefefefefef", "efefefefefefefefefefef", NULL },
        { "", "a", "aa", "aaaa", "aaaaa", "aaaaaa", "aaaaaaaaaa", NULL },
        { "", "ab", "ababab", "abababab", "abababababab", NULL },
        { "", "abc", "abcabc", "abcabcabcabc", "abcabcabcabcabcabc", "abcabcabcabcabcabcabcabc", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(!matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult classes_match_all_characters_contained_in_the_class() {
    const char* examples[] = { "[abc]", "[a-e]", "[ab]", "[a]", "[0123abcd]", "[a-c0-2+_.]" };
    const char* example_match[][256] = {
        { "a", "b", "c", NULL },
        { "a", "b", "c", "d", "e", NULL },
        { "a", "b", NULL },
        { "a", NULL },
        { "0", "1", "2", "3", "a", "b", "c", "d", NULL },
        { "a", "b", "c", "0", "1", "2", "+", "_", ".", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult classes_dont_match_characters_not_contained_in_the_class() {
    const char* examples[] = { "[abc]", "[a-e]", "[ab]", "[a]", "[0123abcd]", "[a-c0-2+_.]" };
    const char* example_match[][256] = {
        { "", " ", "_", "+", "e", "f", "d", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
        { "", " ", "_", "+", "r", "f", "z", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
        { "", "c", " ", "_", "+", "e", "f", "d", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
        { "", "b", "c", " ", "_", "+", "e", "f", "d", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
        { "", " ", "_", "+", "e", "f", "z", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
        { "", " ", "9", "5", "e", "f", "d", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(!matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult inverted_classes_match_all_characters_not_contained_in_the_class() {
    const char* examples[] = { "[^abc]", "[^a-e]", "[^ab]", "[^a]", "[^0123abcd]", "[^a-c0-2+_.]" };
    const char* example_match[][256] = {
        { " ", "_", "+", "e", "f", "d", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
        { " ", "_", "+", "r", "f", "z", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
        { "c", " ", "_", "+", "e", "f", "d", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
        { "b", "c", " ", "_", "+", "e", "f", "d", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
        { " ", "_", "+", "e", "f", "z", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
        { " ", "9", "5", "e", "f", "d", "k", "A", "B", "C", "D", "L", "K", "q", "Q", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult inverted_classes_dont_match_characters_contained_in_the_class() {
    const char* examples[] = { "[^abc]", "[^a-e]", "[^ab]", "[^a]", "[^0123abcd]", "[^a-c0-2+_.]" };
    const char* example_match[][256] = {
        { "", "a", "b", "c", NULL },
        { "", "a", "b", "c", "d", "e", NULL },
        { "", "a", "b", NULL },
        { "", "a", NULL },
        { "", "0", "1", "2", "3", "a", "b", "c", "d", NULL },
        { "", "a", "b", "c", "0", "1", "2", "+", "_", ".", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(!matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult predefined_classes_match_all_characters_contained_in_the_class() {
    const char* examples[] = {
        "$", ".", "\\s", "\\w", "\\d", "\\a", "\\l", "\\u", "\\p", "\\g",
                  "\\S", "\\W", "\\D", "\\A", "\\L", "\\U", "\\P", "\\G",
    };
    const char* example_match[][256] = {
        { "\n", NULL },
        { "a", "b", "c", "0", "1", "2", "-", ".", "+", "l", "A", "B", "Z", "Y", NULL },
        
        { " ", "\t", "\n", NULL },
        { "a", "b", "y", "z", "A", "B", "Y", "Z", "0", "1", "8", "9", "_", NULL },
        { "0", "1", "2", "3", "5", "6", "8", "9", NULL },
        { "a", "b", "y", "z", "A", "B", "Y", "Z", NULL },
        { "a", "b", "e", "i", "o", "u", "y", "z", NULL },
        { "A", "B", "E", "I", "O", "U", "Y", "Z", NULL },
        { "a", "b", "c", "0", "1", "2", "-", ".", "+", "l", "A", "B", "Z", "Y", " ", NULL },
        { "a", "b", "c", "0", "1", "2", "-", ".", "+", "l", "A", "B", "Z", "Y", NULL },

        { "a", "b", "c", "A", "B", "C", "D", "0", "1", "8", "9", "-", "+", ".", NULL },
        { " ", "\t", "-", ".", ",", ":", "+", NULL },
        { "a", "b", "y", "z", "A", "B", "Y", "Z", "-", ".", ",", ":", "+", NULL },
        { "0", "1", "8", "9", "4", "-", ".", ",", ":", "+", NULL },
        { "A", "B", "Y", "Z", "0", "1", "8", "9", "-", ".", ",", ":", "+", NULL },
        { "a", "b", "y", "z", "0", "1", "8", "9", "-", ".", ",", ":", "+", NULL },
        { "\n", "\t", "\b", "\e", NULL },
        { "\n", "\t", "\b", "\e", " ", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult predefined_classes_dont_match_characters_not_contained_in_the_class() {
    const char* examples[] = {
        "$", ".", "\\s", "\\w", "\\d", "\\a", "\\l", "\\u", "\\p", "\\g",
                  "\\S", "\\W", "\\D", "\\A", "\\L", "\\U", "\\P", "\\G",
    };
    const char* example_match[][256] = {
        { "a", "b", "c", "0", "1", "2", "-", ".", "+", "l", "A", "B", "Z", "Y", NULL },
        { "\n", NULL },

        { "a", "b", "c", "A", "B", "C", "D", "0", "1", "8", "9", "-", "+", ".", NULL },
        { " ", "\t", "-", ".", ",", ":", "+", NULL },
        { "a", "b", "y", "z", "A", "B", "Y", "Z", "-", ".", ",", ":", "+", NULL },
        { "0", "1", "8", "9", "4", "-", ".", ",", ":", "+", NULL },
        { "A", "B", "Y", "Z", "0", "1", "8", "9", "-", ".", ",", ":", "+", NULL },
        { "a", "b", "y", "z", "0", "1", "8", "9", "-", ".", ",", ":", "+", NULL },
        { "\n", "\t", "\b", "\e", NULL },
        { "\n", "\t", "\b", "\e", " ", NULL },
        
        { " ", "\t", "\n", NULL },
        { "a", "b", "y", "z", "A", "B", "Y", "Z", "0", "1", "8", "9", "_", NULL },
        { "0", "1", "2", "3", "5", "6", "8", "9", NULL },
        { "a", "b", "y", "z", "A", "B", "Y", "Z", NULL },
        { "a", "b", "e", "i", "o", "u", "y", "z", NULL },
        { "A", "B", "E", "I", "O", "U", "Y", "Z", NULL },
        { "a", "b", "c", "0", "1", "2", "-", ".", "+", "l", "A", "B", "Z", "Y", " ", NULL },
        { "a", "b", "c", "0", "1", "2", "-", ".", "+", "l", "A", "B", "Z", "Y", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(!matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult round_bracets_contain_a_group() {
    const char* examples[] = { "(a|b|c)(d|e|f)", "(a+b){2}", "((ab|cd)|(a|b))ef", "((ab)+c*)+" };
    const char* example_match[][256] = {
        { "ad", "ae", "af", "bd", "be", "bf", "cd", "ce", "cf", NULL },
        { "abab", "aaaaaabab", "abaaaaaab", "aaaaabaaaaaaaab", NULL },
        { "abef", "cdef", "aef", "bef", NULL },
        { "abc", "ab", "abababcabababcabcabcab", NULL },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            ASSERT_EX_MA(matchRegex(regex, example_match[e][m], NULL), e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult starts_with_regex_returns_the_maximum_matching_length_or_negative_one() {
    const char* examples[] = { "abcd", "a*", "(a*bc)+a(bc)+", };
    const char* example_match[][256] = {
        { "abcdefghij", "", "abc", NULL },
        { "aaaaab", "baa", "a", "aaaaaaaaab", "b", NULL },
        { "aaaabcbcbcabcbc...", "bcbcabcbc", "abca", "abcabcabcl", NULL },
    };
    const int example_len[][256] = {
        { 4, -1, -1 },
        { 5, 0, 1, 9, 0 },
        { 15, 9, -1, 9 },
    };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingRegex(examples[e]);
        for(int m = 0; example_match[e][m] != NULL; m++) {
            int len = 0;
            startsWithRegex(regex, example_match[e][m], &len, NULL);
            ASSERT_EX_MA(len == example_len[e][m], e, m);
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult match_regex_returns_the_first_exit_taken_or_negative_one() {
    const char* examples[] = { "abc", "abcd", "a*", "(a*bc)+a(bc)+" };
    const char* example_match[] = {
        "abc", "abcd", "aaaaaaaaa", "aaaaabcabcbc", "aaabcbcaabcbc",
    };
    const int example_len[] = {
        0, 1, 2, 3, -1,
    };
    Regex regex = compileMultiMatchingRegex(LEN(examples), examples);
    for(int m = 0; m < LEN(example_match); m++) {
        int exit_num = 0;
        matchRegex(regex, example_match[m], &exit_num);
        ASSERT_EX(exit_num == example_len[m], m);
    }
    disposeRegex(regex);
    return SUCCESS;
}

TestResult starts_with_regex_returns_the_first_exit_taken_or_negative_one() {
    const char* examples[] = { "abc", "abcd", "a+", "(a*bc)+a(bc)+" };
    const char* example_match[] = {
        "abcwww", "abcdwww", "aaaaaaaaawww", "aaaaabcabcbcwww", "www"
    };
    const int example_len[] = {
        0, 1, 2, 3, -1,
    };
    Regex regex = compileMultiMatchingRegex(LEN(examples), examples);
    for(int m = 0; m < LEN(example_match); m++) {
        int exit_num = 0;
        startsWithRegex(regex, example_match[m], NULL, &exit_num);
        ASSERT_EX(exit_num == example_len[m], m);
    }
    disposeRegex(regex);
    return SUCCESS;
}

TestResult strings_match_themselfs() {
    const char* examples[] = { "te?s+t*", "[abc]", "d{2}ef", "q+ed*[abc]*" };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingString(examples[e]);
        ASSERT_EX(matchRegex(regex, examples[e], NULL), e);
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult strings_dont_match_other_strings() {
    const char* examples[] = { "te?s+t*", "[abc]", "d{2}ef", "q+ed*[abc]*" };
    for(int e = 0; e < LEN(examples); e++) {
        Regex regex = compileMatchingString(examples[e]);
        for(int m = 0; m < LEN(examples); m++) {
            if(m != e) {
                ASSERT_EX_MA(!matchRegex(regex, examples[m], NULL), e, m);
            }
        }
        disposeRegex(regex);
    }
    return SUCCESS;
}

TestResult string_match_regex_returns_the_first_exit_taken_or_negative_one() {
    const char* examples[] = { "abc", "abcd", "a", "abcabc" };
    const char* example_match[] = {
        "abc", "abcd", "a", "abcabc", "aaabcbcaabcbc",
    };
    const int example_len[] = {
        0, 1, 2, 3, -1,
    };
    Regex regex = compileMultiMatchingStrings(LEN(examples), examples);
    for(int m = 0; m < LEN(example_match); m++) {
        int exit_num = 0;
        matchRegex(regex, example_match[m], &exit_num);
        ASSERT_EX(exit_num == example_len[m], m);
    }
    disposeRegex(regex);
    return SUCCESS;
}

TestResult string_starts_with_regex_returns_the_first_exit_taken_or_negative_one() {
    const char* examples[] = { "abc", "abcd", "a", "abcabc" };
    const char* example_match[] = {
        "abcef", "abcdef", "aef", "abcabcef", "efaaabcbcaabcbc",
    };
    const int example_len[] = {
        0, 1, 2, 3, -1,
    };
    Regex regex = compileMultiMatchingStrings(LEN(examples), examples);
    for(int m = 0; m < LEN(example_match); m++) {
        int exit_num = 0;
        startsWithRegex(regex, example_match[m], NULL, &exit_num);
        ASSERT_EX(exit_num == example_len[m], m);
    }
    disposeRegex(regex);
    return SUCCESS;
}

TestResult string_regex_match_regex_returns_the_first_exit_taken_or_negative_one() {
    const char* examples[] = { "abc", "abcd", "a*", "abc(abc)+" };
    const bool example_regex[] = { false, false, true, true };
    const char* example_match[] = {
        "abc", "abcd", "aaaaaaa", "abcabcabcabc", "aaabcbcaabcbc",
    };
    const int example_len[] = {
        0, 1, 2, 3, -1,
    };
    Regex regex = compileMultiMatchingStringsAndRegex(LEN(examples), example_regex, examples);
    for(int m = 0; m < LEN(example_match); m++) {
        int exit_num = 0;
        matchRegex(regex, example_match[m], &exit_num);
        ASSERT_EX(exit_num == example_len[m], m);
    }
    disposeRegex(regex);
    return SUCCESS;
}

TestResult string_regex_starts_with_regex_returns_the_first_exit_taken_or_negative_one() {
    const char* examples[] = { "abc", "abcd", "a+", "abc(abc)+" };
    const bool example_regex[] = { false, false, true, true };
    const char* example_match[] = {
        "abcef", "abcdef", "aef", "abcabcef", "efaaabcbcaabcbc",
    };
    const int example_len[] = {
        0, 1, 2, 3, -1,
    };
    Regex regex = compileMultiMatchingStringsAndRegex(LEN(examples), example_regex, examples);
    for(int m = 0; m < LEN(example_match); m++) {
        int exit_num = 0;
        startsWithRegex(regex, example_match[m], NULL, &exit_num);
        ASSERT_EX(exit_num == example_len[m], m);
    }
    disposeRegex(regex);
    return SUCCESS;
}

static Test tests[] = {
    TEST(cheching_a_valid_regex_returns_negative_one),
    TEST(checking_a_invalid_regex_returns_the_error_location),
    TEST(compiling_a_valid_regex_returns_non_null),
    TEST(compiling_a_invalid_regex_returns_null),
    TEST(simple_strings_match_themselfs),
    TEST(simple_strings_dont_match_other_strings),
    TEST(using_the_pipe_matches_one_of_the_options),
    TEST(using_the_pipe_doesnt_match_a_string_not_in_the_options),
    TEST(using_the_star_matches_zero_or_more_of_the_previous_group),
    TEST(using_the_plus_matches_one_or_more_of_the_previous_group),
    TEST(using_the_plus_doesnt_match_zero_repetitions),
    TEST(using_the_question_mark_matches_zero_or_one_of_the_previous_group),
    TEST(using_the_question_mark_doesnt_match_more_than_one_of_the_previous_group),
    TEST(using_the_curly_bacets_can_specify_the_repetitions_of_the_previous_group),
    TEST(using_the_curly_bacets_can_specify_a_range_of_repetitions_of_the_previous_group),
    TEST(using_the_curly_bacets_doesnt_match_repetitions_outside_the_range),
    TEST(classes_match_all_characters_contained_in_the_class),
    TEST(classes_dont_match_characters_not_contained_in_the_class),
    TEST(inverted_classes_match_all_characters_not_contained_in_the_class),
    TEST(inverted_classes_dont_match_characters_contained_in_the_class),
    TEST(predefined_classes_match_all_characters_contained_in_the_class),
    TEST(predefined_classes_dont_match_characters_not_contained_in_the_class),
    TEST(round_bracets_contain_a_group),
    TEST(starts_with_regex_returns_the_maximum_matching_length_or_negative_one),
    TEST(match_regex_returns_the_first_exit_taken_or_negative_one),
    TEST(starts_with_regex_returns_the_first_exit_taken_or_negative_one),
    TEST(strings_match_themselfs),
    TEST(strings_dont_match_other_strings),
    TEST(string_match_regex_returns_the_first_exit_taken_or_negative_one),
    TEST(string_starts_with_regex_returns_the_first_exit_taken_or_negative_one),
    TEST(string_regex_match_regex_returns_the_first_exit_taken_or_negative_one),
    TEST(string_regex_starts_with_regex_returns_the_first_exit_taken_or_negative_one),
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
                if(result.exm != -1) {
                    if(result.mat != -1) {
                        fprintf(stderr, "\e[31mFailed\e[m test '%s' (%i, %i)\n  \e[31m|\e[m %s\n", tests[t].name, result.exm, result.mat, result.msg);
                    } else {
                        fprintf(stderr, "\e[31mFailed\e[m test '%s' (%i)\n  \e[31m|\e[m %s\n", tests[t].name, result.exm, result.msg);
                    }
                } else {
                    fprintf(stderr, "\e[31mFailed\e[m test '%s'\n  \e[31m|\e[m %s\n", tests[t].name, result.msg);
                }
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
