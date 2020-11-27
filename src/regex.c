
#include <stdlib.h>
#include <stdio.h>

#include "regex-nfa.h"
#include "regex-dfa.h"
#include "regex.h"

Regex compileMatchingRegex(const char* regex_string) {
    RegexNodeSet nodes = REGEX_NODE_SET_INIT;
    RegexNode start = REGEX_NODE_INIT;
    RegexNodeRef start_ref = pushNodeToRegexNodeSet(&nodes, start);
    const char* end_pos;
    RegexNodeRef last_node = parseRegexGroup(&nodes, start_ref, regex_string, &end_pos, false);
    if(last_node < 0 || *end_pos != 0) {
        freeNodes(&nodes);
        return NULL;
    } else {
        nodes.nodes[last_node].exit_num = 0;
        Regex ret = compileRegexToStateMachine(&nodes, start_ref);
        freeNodes(&nodes);
        return ret;
    }
}

Regex compileMatchingString(const char* string) {
    RegexNodeSet nodes = REGEX_NODE_SET_INIT;
    RegexNode start = REGEX_NODE_INIT;
    RegexNodeRef start_ref = pushNodeToRegexNodeSet(&nodes, start);
    RegexNodeRef last_node = stringToNfa(&nodes, start_ref, string);
    nodes.nodes[last_node].exit_num = 0;
    Regex ret = compileRegexToStateMachine(&nodes, start_ref);
    freeNodes(&nodes);
    return ret;
}

Regex compileMultiMatchingRegex(int num_regex, const char* const* regex_strings) {
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
        RegexNodeRef last_node = parseRegexGroup(&nodes, regex_start_ref, regex_strings[i], &end_pos, false);
        if(last_node < 0 || *end_pos != 0) {
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

Regex compileMultiMatchingStrings(int num_strings, const char* const* strings) {
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
        RegexNodeRef last_node = stringToNfa(&nodes, regex_start_ref, strings[i]);
        nodes.nodes[last_node].exit_num = i;
    }
    Regex ret = compileRegexToStateMachine(&nodes, start_ref);
    freeNodes(&nodes);
    return ret;
}

Regex compileMultiMatchingStringsAndRegex(int num_patterns, const bool* is_regex, const char* const* patterns) {
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
            last_node = parseRegexGroup(&nodes, regex_start_ref, patterns[i], &end_pos, false);
            if(last_node < 0 || *end_pos != 0) {
                freeNodes(&nodes);
                return NULL;
            }
        } else {
            last_node = stringToNfa(&nodes, regex_start_ref, patterns[i]);
        }
        nodes.nodes[last_node].exit_num = i;
    }
    Regex ret = compileRegexToStateMachine(&nodes, start_ref);
    freeNodes(&nodes);
    return ret;
}

bool startsWithRegex(Regex regex, const char* string, int* len_out, int* exit_num) {
    int last_len = -1;
    int last_exit = -1;
    int state = 0;
    for(int len = 0;;string++, len++) {
        RegexStateTransition transition = regex->states[state][(unsigned char)*string];
        switch (transition.state_type) {
        case REGEX_STATE_DEADEND:
            if(len_out != NULL) {
                *len_out = last_len;
            }
            if(exit_num != NULL) {
                *exit_num = last_exit;
            }
            if(len == -1) {
                return false;
            } else {
                return true;
            }
            break;
        case REGEX_STATE_NEXT:
            if(regex->states[state][0].state_type == REGEX_STATE_END) {
                last_len = len;
                last_exit = regex->states[state][0].end_point;
            }
            state = transition.next_state;
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
}

bool matchRegex(Regex regex, const char* string, int* exit_num) {
    int state = 0;
    for(;;string++) {
        RegexStateTransition transition = regex->states[state][(unsigned char)*string];
        switch (transition.state_type) {
        case REGEX_STATE_DEADEND:
            if(exit_num != NULL) {
                *exit_num = -1;
            }
            return false;
            break;
        case REGEX_STATE_NEXT:
            state = transition.next_state;
            break;
        case REGEX_STATE_END:
            if(*string == 0) {
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
}

void disposeRegex(Regex regex) {
    free(regex);
}
