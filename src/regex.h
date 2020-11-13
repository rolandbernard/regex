#ifndef _REGEX_H_
#define _REGEX_H_

#include <stdbool.h>

typedef enum {
    REGEX_STATE_DEADEND = 0,
    REGEX_STATE_NEXT,
    REGEX_STATE_END,
} RegexStateTransitionType;

typedef struct {
    RegexStateTransitionType state_type;
    union {
        int next_state;
        int end_point;
    };
} RegexStateTransition;

typedef RegexStateTransition (*Regex)[256];

Regex compileMatchingRegex(const char* regex_string);

Regex compileMultiMatchingRegex(int num_regex, const char* const* regex_strings);

bool startsWithRegex(Regex regex, const char* string, int* len, int* exit_num);

bool matchRegex(Regex regex, const char* string, int* exit_num);

void disposeRegex(Regex regex);

#endif