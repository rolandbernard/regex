#ifndef _REGEX_H_
#define _REGEX_H_

#include <stdbool.h>

typedef struct {
    bool is_end;
    union {
        int next_state;
        int end_point;
    };
} RegExState;

typedef RegExState (*RegEx)[256];

RegEx compileMatchingRegEx(const char* regex_string);

RegEx compileMultiMatchingRegEx(int num_regex, const char* const* regex_strings);

bool startsWithRegex(RegEx regex, const char* string, int* len, int* exit_num);

void disposeRegEx(RegEx regex);

#endif