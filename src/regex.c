
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
        // TODO: this is only here for debuging
        // fprintf(stderr, "NFA\n");
        // for(int i = 0; i < nodes.node_count; i++) {
        //     if(nodes.nodes[i].exit_num != -1) {
        //         putc('*', stderr);
        //     } else {
        //         putc(' ', stderr);
        //     }
        //     fprintf(stderr, "%i: ", i);
        //     for(int j = 0; j < nodes.nodes[i].connection_count; j++) {
        //         if(nodes.nodes[i].connections[j].class_str == NULL) {
        //             fprintf(stderr, "%i, ", nodes.nodes[i].connections[j].next_node);
        //         } else {
        //             fwrite(nodes.nodes[i].connections[j].class_str, 1, nodes.nodes[i].connections[j].class_len, stderr);
        //             fprintf(stderr, ":%i, ", nodes.nodes[i].connections[j].next_node);
        //         }
        //     }
        //     fprintf(stderr, "\n");
        // }
        // fprintf(stderr, "\n");
        Regex ret = compileRegexToStateMachine(&nodes, start_ref);
        // fprintf(stderr, "DFA\n");
        // int num_states = 1;
        // for(int i = 0; i < num_states; i++) {
        //     if(ret[i][255].state_type == REGEX_STATE_END) {
        //         putc('*', stderr);
        //     } else {
        //         putc(' ', stderr);
        //     }
        //     fprintf(stderr, "%i: ", i);
        //     for(int j = 0; j < 256; j++) {
        //         if (ret[i][j].state_type == REGEX_STATE_NEXT) {
        //             if (isgraph(j) || j == ' ') {
        //                 fprintf(stderr, "%c:%i, ", (char)j, ret[i][j].next_state);
        //             }
        //             if (ret[i][j].next_state + 1 > num_states) {
        //                 num_states = ret[i][j].next_state + 1;
        //             }
        //         }
        //     }
        //     fprintf(stderr, "\n");
        // }
        // fprintf(stderr, "\n");
        // fprintf(stderr, "\n");
        freeNodes(&nodes);
        return ret;
    }
}

Regex compileMultiMatchingRegex(int num_regex, const char* const* regex_strings) {
    return NULL;
}

bool startsWithRegex(Regex regex, const char* string, int* len_out, int* exit_num) {
    return false;
    int last_len = -1;
    int last_exit = -1;
    int state = 0;
    for(int len = 0;;string++, len++) {
        RegexStateTransition transition = regex[state][(unsigned char)*string];
        switch (transition.state_type) {
        case REGEX_STATE_DEADEND:
            if(len == -1) {
                return false;
            } else {
                if(len_out != NULL) {
                    *len_out = last_len;
                }
                if(exit_num != NULL) {
                    *exit_num = last_exit;
                }
                return true;
            }
            break;
        case REGEX_STATE_NEXT:
            if(regex[state][0].state_type == REGEX_STATE_END) {
                last_len = len;
                last_exit = regex[state][0].end_point;
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
        RegexStateTransition transition = regex[state][(unsigned char)*string];
        switch (transition.state_type) {
        case REGEX_STATE_DEADEND:
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
                return false;
            }
            break;
        }
    }
}

void disposeRegex(Regex regex) {
    free(regex);
}
