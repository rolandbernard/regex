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

typedef RegexStateTransition (*Regex)[256];

#endif