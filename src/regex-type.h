#ifndef _REGEX_TYPE_H_
#define _REGEX_TYPE_H_

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

#define REGEX_NUM_CHARS 256

typedef struct {
    int num_states;
    RegexStateTransition states[][REGEX_NUM_CHARS + 1];
} RegexTransitionTable;

typedef RegexTransitionTable* Regex;

#endif