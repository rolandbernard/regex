
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "regex-nfa.h"
#include "regex-dfa.h"
#include "regex.h"

int getRegexErrorLocationN(const char* regex_string, int len) {
    RegexNodeSet nodes = REGEX_NODE_SET_INIT;
    RegexNode start = REGEX_NODE_INIT;
    RegexNodeRef start_ref = pushNodeToRegexNodeSet(&nodes, start);
    const char* end_pos;
    RegexNodeRef last_node = parseRegexGroup(&nodes, start_ref, regex_string, len, &end_pos, false);
    if(last_node < 0 || end_pos != regex_string + len) {
        freeNodes(&nodes);
        return (int)(end_pos - regex_string);
    } else {
        freeNodes(&nodes);
        return -1;
    }
}

int getRegexErrorLocation(const char* regex_string) {
    return getRegexErrorLocationN(regex_string, strlen(regex_string));
}

Regex compileMatchingRegexN(const char* regex_string, int len) {
    RegexNodeSet nodes = REGEX_NODE_SET_INIT;
    RegexNode start = REGEX_NODE_INIT;
    RegexNodeRef start_ref = pushNodeToRegexNodeSet(&nodes, start);
    const char* end_pos;
    RegexNodeRef last_node = parseRegexGroup(&nodes, start_ref, regex_string, len, &end_pos, false);
    if(last_node < 0 || end_pos != regex_string + len) {
        freeNodes(&nodes);
        return NULL;
    } else {
        nodes.nodes[last_node].exit_num = 0;
        Regex ret = compileRegexToStateMachine(&nodes, start_ref);
        freeNodes(&nodes);
        return ret;
    }
}

Regex compileMatchingRegex(const char* regex_string) {
    return compileMatchingRegexN(regex_string, strlen(regex_string));
}

Regex compileMatchingStringN(const char* string, int len) {
    RegexNodeSet nodes = REGEX_NODE_SET_INIT;
    RegexNode start = REGEX_NODE_INIT;
    RegexNodeRef start_ref = pushNodeToRegexNodeSet(&nodes, start);
    RegexNodeRef last_node = stringToNfa(&nodes, start_ref, string, len);
    nodes.nodes[last_node].exit_num = 0;
    Regex ret = compileRegexToStateMachine(&nodes, start_ref);
    freeNodes(&nodes);
    return ret;
}

Regex compileMatchingString(const char* string) {
    return compileMatchingStringN(string, strlen(string));
}

Regex compileMultiMatchingRegexN(int num_regex, const char* const* regex_strings, const int* lengths) {
    RegexNodeSet nodes = REGEX_NODE_SET_INIT;
    RegexNode empty_node = REGEX_NODE_INIT;
    RegexNodeRef start_ref = pushNodeToRegexNodeSet(&nodes, empty_node);
    for(int i = 0; i < num_regex; i++) {
        RegexNodeRef regex_start_ref = pushNodeToRegexNodeSet(&nodes, empty_node);
        RegexConnection connection = {
            .class_str = NULL,
            .class_len = 0,
            .next_node = regex_start_ref,
        };
        pushConnectionToRegexNode(&nodes.nodes[start_ref], connection);
        const char* end_pos;
        RegexNodeRef last_node = parseRegexGroup(&nodes, regex_start_ref, regex_strings[i], lengths[i], &end_pos, false);
        if(last_node < 0 || end_pos != regex_strings[i] + lengths[i]) {
            freeNodes(&nodes);
            return NULL;
        } else {
            nodes.nodes[last_node].exit_num = i;
        }
    }
    Regex ret = compileRegexToStateMachine(&nodes, start_ref);
    freeNodes(&nodes);
    return ret;
}

Regex compileMultiMatchingRegex(int num_regex, const char* const* regex_strings) {
    int lengths[num_regex];
    for(int i = 0; i < num_regex; i++) {
        lengths[i] = strlen(regex_strings[i]);
    }
    return compileMultiMatchingRegexN(num_regex, regex_strings, lengths);
}

Regex compileMultiMatchingStringsN(int num_strings, const char* const* strings, const int* lengths) {
    RegexNodeSet nodes = REGEX_NODE_SET_INIT;
    RegexNode empty_node = REGEX_NODE_INIT;
    RegexNodeRef start_ref = pushNodeToRegexNodeSet(&nodes, empty_node);
    for(int i = 0; i < num_strings; i++) {
        RegexNodeRef regex_start_ref = pushNodeToRegexNodeSet(&nodes, empty_node);
        RegexConnection connection = {
            .class_str = NULL,
            .class_len = 0,
            .next_node = regex_start_ref,
        };
        pushConnectionToRegexNode(&nodes.nodes[start_ref], connection);
        RegexNodeRef last_node = stringToNfa(&nodes, regex_start_ref, strings[i], lengths[i]);
        nodes.nodes[last_node].exit_num = i;
    }
    Regex ret = compileRegexToStateMachine(&nodes, start_ref);
    freeNodes(&nodes);
    return ret;
}

Regex compileMultiMatchingStrings(int num_strings, const char* const* strings) {
    int lengths[num_strings];
    for(int i = 0; i < num_strings; i++) {
        lengths[i] = strlen(strings[i]);
    }
    return compileMultiMatchingStringsN(num_strings, strings, lengths);
}

Regex compileMultiMatchingStringsAndRegexN(int num_patterns, const bool* is_regex, const char* const* patterns, const int* lengths) {
    RegexNodeSet nodes = REGEX_NODE_SET_INIT;
    RegexNode empty_node = REGEX_NODE_INIT;
    RegexNodeRef start_ref = pushNodeToRegexNodeSet(&nodes, empty_node);
    for(int i = 0; i < num_patterns; i++) {
        RegexNodeRef regex_start_ref = pushNodeToRegexNodeSet(&nodes, empty_node);
        RegexConnection connection = {
            .class_str = NULL,
            .class_len = 0,
            .next_node = regex_start_ref,
        };
        pushConnectionToRegexNode(&nodes.nodes[start_ref], connection);
        RegexNodeRef last_node;
        if(is_regex[i]) {
            const char* end_pos;
            last_node = parseRegexGroup(&nodes, regex_start_ref, patterns[i], lengths[i], &end_pos, false);
            if(last_node < 0 || end_pos != patterns[i] + lengths[i]) {
                freeNodes(&nodes);
                return NULL;
            }
        } else {
            last_node = stringToNfa(&nodes, regex_start_ref, patterns[i], lengths[i]);
        }
        nodes.nodes[last_node].exit_num = i;
    }
    Regex ret = compileRegexToStateMachine(&nodes, start_ref);
    freeNodes(&nodes);
    return ret;
}

Regex compileMultiMatchingStringsAndRegex(int num_patterns, const bool* is_regex, const char* const* patterns) {
    int lengths[num_patterns];
    for(int i = 0; i < num_patterns; i++) {
        lengths[i] = strlen(patterns[i]);
    }
    return compileMultiMatchingStringsAndRegexN(num_patterns, is_regex, patterns, lengths);
}

bool startsWithRegexN(Regex regex, const char* string, int size, int* len_out, int* exit_num) {
    int last_len = -1;
    int last_exit = -1;
    int state = 0;
    int len;
    for(len = 0; size != 0; string++, len++, size--) {
        RegexStateTransition transition = regex->states[state][(unsigned char)*string];
        switch (transition.state_type) {
        case REGEX_STATE_DEADEND:
            goto break_for_loop;
            break;
        case REGEX_STATE_NEXT:
            if(regex->states[state][REGEX_STATE_NEXT].state_type == REGEX_STATE_END) {
                last_len = len;
                last_exit = regex->states[state][REGEX_STATE_NEXT].end_point;
            }
            state = transition.next_state;
            break;
        case REGEX_STATE_END:
            goto break_for_loop;
            break;
        }
    }
    RegexStateTransition transition;
break_for_loop:
    transition = regex->states[state][REGEX_NUM_CHARS];
    switch (transition.state_type) {
    default:
    case REGEX_STATE_NEXT:
    case REGEX_STATE_DEADEND:
        if(len_out != NULL) {
            *len_out = last_len;
        }
        if(exit_num != NULL) {
            *exit_num = last_exit;
        }
        if(last_len == -1) {
            return false;
        } else {
            return true;
        }
        break;
    case REGEX_STATE_END:
        if(len_out != NULL) {
            *len_out = len;
        }
        if(exit_num != NULL) {
            *exit_num = transition.end_point;
        }
        return true;
        break;
    }
}

bool startsWithRegex(Regex regex, const char* string, int* len_out, int* exit_num) {
    return startsWithRegexN(regex, string, strlen(string), len_out, exit_num);
}

bool matchRegexN(Regex regex, const char* string, int size, int* exit_num) {
    int state = 0;
    for(; size != 0; string++, size--) {
        RegexStateTransition transition = regex->states[state][(unsigned char)*string];
        switch (transition.state_type) {
        case REGEX_STATE_DEADEND:
            goto break_for_loop;
            break;
        case REGEX_STATE_NEXT:
            state = transition.next_state;
            break;
        case REGEX_STATE_END:
            goto break_for_loop;
            break;
        }
    }
    RegexStateTransition transition;
break_for_loop:
    transition = regex->states[state][REGEX_NUM_CHARS];
    switch (transition.state_type) {
    default:
    case REGEX_STATE_NEXT:
    case REGEX_STATE_DEADEND:
        if(exit_num != NULL) {
            *exit_num = -1;
        }
        return false;
        break;
    case REGEX_STATE_END:
        if (size == 0) {
            if(exit_num != NULL) {
                *exit_num = transition.end_point;
            }
            return true;
        } else {
            if(exit_num != NULL) {
                *exit_num = -1;
            }
            return false;
        }
        break;
    }
}

bool matchRegex(Regex regex, const char* string, int* exit_num) {
    return matchRegexN(regex, string, strlen(string), exit_num);
}

void disposeRegex(Regex regex) {
    free(regex);
}

void printRegexDfa(Regex reg) {
    for(int i = 0; i < reg->num_states; i++) {
        if(reg->states[i][0].state_type == REGEX_STATE_END) {
            fprintf(stderr, "%i*%i: ", reg->states[i][0].end_point, i);
        } else {
            fprintf(stderr, "  %i: ", i);
        }
        for(int j = 1; j < 256; j++) {
            if(isprint(j) && reg->states[i][j].state_type == REGEX_STATE_NEXT) {
                fprintf(stderr, "%c:%i ", (char)j, reg->states[i][j].next_state);
            }
        }
        fprintf(stderr, "\n");
    }
}

