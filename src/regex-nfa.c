
#include <ctype.h>

#include "regex-nfa.h"

void pushConnectionToRegexNode(RegexNode* node, RegexConnection conn) {
    if(node->connection_capacity == node->connection_count) {
        if(node->connection_capacity == 0) {
            node->connection_capacity = 4;
        } else {
            node->connection_capacity *= 2;
        }
        node->connections = (RegexConnection*)realloc(node->connections, sizeof(RegexConnection) * node->connection_capacity);
    }
    node->connections[node->connection_count] = conn;
    node->connection_count++;
}

RegexNodeRef pushNodeToRegexNodeSet(RegexNodeSet* node_set, RegexNode node) {
    if(node_set->node_capacity == node_set->node_count) {
        if(node_set->node_capacity == 0) {
            node_set->node_capacity = 16;
        } else {
            node_set->node_capacity *= 2;
        }
        node_set->nodes = (RegexNode*)realloc(node_set->nodes, sizeof(RegexNode) * node_set->node_capacity);
    }
    node_set->nodes[node_set->node_count] = node;
    return node_set->node_count++;
}

void freeNodes(RegexNodeSet* node_set) {
    for(int n = 0; n < node_set->node_count; n++) {
        free(node_set->nodes[n].connections);
    }
    free(node_set->nodes);
}

static void recursiveNodeCopy(RegexNodeSet* nodes, RegexNodeRef node, RegexNodeRef* new_nodes, RegexNodeRef offset, RegexNodeRef max) {
    for (int c = 0; c < nodes->nodes[node].connection_count; c++) {
        RegexConnection connection = nodes->nodes[node].connections[c];
        if (connection.next_node >= offset && connection.next_node < max) {
            if (new_nodes[connection.next_node - offset] == -1) {
                RegexNode node = REGEX_NODE_INIT;
                new_nodes[connection.next_node - offset] = pushNodeToRegexNodeSet(nodes, node);
                recursiveNodeCopy(nodes, connection.next_node, new_nodes, offset, max);
            }
            connection.next_node = new_nodes[connection.next_node - offset];
            pushConnectionToRegexNode(&nodes->nodes[new_nodes[node - offset]], connection);
        }
    }
}

static RegexNodeRef cloneLastNodes(RegexNodeSet* nodes, RegexNodeRef start, RegexNodeRef end) {
    RegexNodeRef new_nodes[nodes->node_count - start];
    for(int i = 0; i < nodes->node_count - start; i++) {
        new_nodes[i] = -1;
    }
    new_nodes[0] = end;
    recursiveNodeCopy(nodes, start, new_nodes, start, nodes->node_count);
    return new_nodes[end - start];
}

RegexNodeRef parseRegexGroup(RegexNodeSet* nodes, RegexNodeRef start, const char* regex, int size, const char** end_pos, bool inside_or) {
    RegexNodeRef last_node = -1;
    RegexNodeRef current_node = start;
    while(size > 0 && *regex != ')' && (!inside_or || *regex != '|')) {
        switch (*regex) {
        case '[': {
            int len = 1;
            while(size - len > 0 && regex[len] != ']') {
                if(regex[len] == '\\') {
                    len++;
                }
                len++;
            }
            if(size - len == 0) {
                *end_pos = regex;
                return -1;
            } else {
                len++;
                RegexNode node = REGEX_NODE_INIT;
                RegexNodeRef node_ref = pushNodeToRegexNodeSet(nodes, node);
                RegexConnection connection = {
                    .class_str = regex,
                    .class_len = len,
                    .next_node = node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], connection);
                last_node = current_node;
                current_node = node_ref;
                regex += len;
                size -= len;
            }
            break;
        }
        case '(': {
            const char* error_pos = regex;
            regex++;
            size--;
            last_node = current_node;
            const char* ending_pos;
            current_node = parseRegexGroup(nodes, current_node, regex, size, &ending_pos, false);
            size -= (ending_pos - regex);
            regex = ending_pos;
            if(current_node == -1 || size == 0 || *regex != ')') {
                *end_pos = error_pos;
                return -1;
            } else {
                regex++;
                size--;
            }
            break;
        }
        case '|': {
            /* 
             * last_node    current_node
             *         v    v
             *      1->O-??-O--2.
             *     /             V    
             *    O--->O-??-O--->O
             *     \            .^ 
             *      3->O-??-O--4      
             */
            RegexNode exit_node = REGEX_NODE_INIT;
            RegexNodeRef exit_node_ref = pushNodeToRegexNodeSet(nodes, exit_node);
            // 1
            RegexNodeRef new_start_ref = pushNodeToRegexNodeSet(nodes, nodes->nodes[start]);
            RegexNode start_replacement = REGEX_NODE_INIT;
            RegexConnection start_connection = {
                .class_str = NULL,
                .class_len = 0,
                .next_node = new_start_ref,
            };
            pushConnectionToRegexNode(&start_replacement, start_connection);
            nodes->nodes[start] = start_replacement;
            // 2
            RegexConnection end_connection = {
                .class_str = NULL,
                .class_len = 0,
                .next_node = exit_node_ref,
            };
            pushConnectionToRegexNode(&nodes->nodes[current_node], end_connection);
            while(*regex == '|') {
                // 3
                RegexNodeRef or_start_node_ref = pushNodeToRegexNodeSet(nodes, exit_node);
                RegexConnection connection = {
                    .class_str = NULL,
                    .class_len = 0,
                    .next_node = or_start_node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[start], connection);
                regex++;
                size--;
                const char* ending_pos;
                RegexNodeRef end_node = parseRegexGroup(nodes, or_start_node_ref, regex, size, &ending_pos, true);
                size -= (ending_pos - regex);
                regex = ending_pos;
                if(end_node == -1) {
                    *end_pos = regex;
                    return -1;
                } else {
                    // 4
                    RegexConnection connection = {
                        .class_str = NULL,
                        .class_len = 0,
                        .next_node = exit_node_ref,
                    };
                    pushConnectionToRegexNode(&nodes->nodes[end_node], connection);
                }
            }
            if(size > 0 && *regex != ')') {
                *end_pos = regex;
                return -1;
            } else {
                last_node = start;
                current_node = exit_node_ref;
            }
            break;
        }
        case '?': {
            if(last_node == -1) {
                *end_pos = regex;
                return -1;
            } else {
                regex++;
                size--;
                /* 
                 * last_node    current_node
                 *         v    v
                 *    O-1->O-??-O
                 *     \___2___-^       
                 */
                // 1
                RegexNodeRef new_last_ref = pushNodeToRegexNodeSet(nodes, nodes->nodes[last_node]);
                RegexNode last_node_replacement = REGEX_NODE_INIT;
                RegexConnection start_connection = {
                    .class_str = NULL,
                    .class_len = 0,
                    .next_node = new_last_ref,
                };
                pushConnectionToRegexNode(&last_node_replacement, start_connection);
                nodes->nodes[last_node] = last_node_replacement;
                // 2
                RegexConnection skip_connection = {
                    .class_str = NULL,
                    .class_len = 0,
                    .next_node = current_node,
                };
                pushConnectionToRegexNode(&nodes->nodes[last_node], skip_connection);
            }
            break;
        }
        case '*': {
            if(last_node == -1) {
                *end_pos = regex;
                return -1;
            } else {
                regex++;
                size--;
                /* 
                 * last_node    current_node
                 *         v    v
                 *    O-1->O-??-O-2->O
                 *     \   ^-_4_/   -^
                 *      \____3_____/       
                 */
                // 1
                RegexNodeRef new_last_ref = pushNodeToRegexNodeSet(nodes, nodes->nodes[last_node]);
                RegexNode last_node_replacement = REGEX_NODE_INIT;
                RegexConnection start_connection = {
                    .class_str = NULL,
                    .class_len = 0,
                    .next_node = new_last_ref,
                };
                pushConnectionToRegexNode(&last_node_replacement, start_connection);
                nodes->nodes[last_node] = last_node_replacement;
                // 2
                RegexNode exit_node = REGEX_NODE_INIT;
                RegexNodeRef exit_node_ref = pushNodeToRegexNodeSet(nodes, exit_node);
                RegexConnection end_connection = {
                    .class_str = NULL,
                    .class_len = 0,
                    .next_node = exit_node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], end_connection);
                // 3
                RegexConnection skip_connection = {
                    .class_str = NULL,
                    .class_len = 0,
                    .next_node = exit_node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[last_node], skip_connection);
                // 4
                RegexConnection repeat_connection = {
                    .class_str = NULL,
                    .class_len = 0,
                    .next_node = new_last_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], repeat_connection);
                current_node = exit_node_ref;
            }
            break;
        }
        case '+': {
            if(last_node == -1) {
                *end_pos = regex;
                return -1;
            } else {
                regex++;
                size--;
                /* 
                 * last_node    current_node
                 *         v    v
                 *    O-1->O-??-O-2->O
                 *         ^-_3_/
                 */
                // 1
                RegexNodeRef new_last_ref = pushNodeToRegexNodeSet(nodes, nodes->nodes[last_node]);
                RegexNode last_node_replacement = REGEX_NODE_INIT;
                RegexConnection start_connection = {
                    .class_str = NULL,
                    .class_len = 0,
                    .next_node = new_last_ref,
                };
                pushConnectionToRegexNode(&last_node_replacement, start_connection);
                nodes->nodes[last_node] = last_node_replacement;
                // 2
                RegexNode exit_node = REGEX_NODE_INIT;
                RegexNodeRef exit_node_ref = pushNodeToRegexNodeSet(nodes, exit_node);
                RegexConnection end_connection = {
                    .class_str = NULL,
                    .class_len = 0,
                    .next_node = exit_node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], end_connection);
                // 3
                RegexConnection repeat_connection = {
                    .class_str = NULL,
                    .class_len = 0,
                    .next_node = new_last_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], repeat_connection);
                current_node = exit_node_ref;
            }
            break;
        }
        case '{': {
            const char* error_pos = regex;
            if(last_node == -1) {
                *end_pos = error_pos;
                return -1;
            } else {
                regex++;
                size--;
                int min = 0;
                while (size > 0 && isspace(*regex)) {
                    regex++;
                    size--;
                }
                while (size > 0 && *regex >= '0' && *regex <= '9') {
                    min *= 10;
                    min += *regex - '0';
                    regex++;
                    size--;
                }
                while (size > 0 && isspace(*regex)) {
                    regex++;
                    size--;
                }
                int max = min;
                if(*regex == ',') {
                    while (size > 0 && isspace(*regex)) {
                        regex++;
                        size--;
                    }
                    regex++;
                    size--;
                    if(*regex == '}') {
                        max = -1;
                    } else {
                        max = 0;
                        while (size > 0 && *regex >= '0' && *regex <= '9') {
                            max *= 10;
                            max += *regex - '0';
                            regex++;
                            size--;
                        }
                    }
                    while (size > 0 && isspace(*regex)) {
                        regex++;
                        size--;
                    }
                }
                if(*regex != '}' || (max != -1 && max < min)) {
                    *end_pos = error_pos;
                    return -1;
                } else {
                    if(min == 0 && max == 0) {
                        *end_pos = error_pos;
                        return -1;
                    }
                    regex++;
                    size--;
                    /* 
                     * last_node    current_node
                     *         v    v
                     *    O-1->O-??-O--->O-??->O--->O-??->O-2->O
                     *                              ^-_3_/
                     */
                    // 1
                    RegexNodeRef new_last_ref = pushNodeToRegexNodeSet(nodes, nodes->nodes[last_node]);
                    RegexNode last_node_replacement = REGEX_NODE_INIT;
                    RegexConnection start_connection = {
                        .class_str = NULL,
                        .class_len = 0,
                        .next_node = new_last_ref,
                    };
                    pushConnectionToRegexNode(&last_node_replacement, start_connection);
                    nodes->nodes[last_node] = last_node_replacement;
                    for(int i = 1; i < min; i++) {
                        RegexNodeRef new_end = cloneLastNodes(nodes, last_node, current_node);
                        last_node = current_node;
                        current_node = new_end;
                    }
                    if(max == -1) {
                        if(min == 0) {
                            // like *
                            RegexNodeRef new_last_ref = pushNodeToRegexNodeSet(nodes, nodes->nodes[last_node]);
                            RegexNode last_node_replacement = REGEX_NODE_INIT;
                            RegexConnection start_connection = {
                                .class_str = NULL,
                                .class_len = 0,
                                .next_node = new_last_ref,
                            };
                            pushConnectionToRegexNode(&last_node_replacement, start_connection);
                            nodes->nodes[last_node] = last_node_replacement;
                            // 2
                            RegexNode exit_node = REGEX_NODE_INIT;
                            RegexNodeRef exit_node_ref = pushNodeToRegexNodeSet(nodes, exit_node);
                            RegexConnection end_connection = {
                                .class_str = NULL,
                                .class_len = 0,
                                .next_node = exit_node_ref,
                            };
                            pushConnectionToRegexNode(&nodes->nodes[current_node], end_connection);
                            RegexConnection skip_connection = {
                                .class_str = NULL,
                                .class_len = 0,
                                .next_node = exit_node_ref,
                            };
                            pushConnectionToRegexNode(&nodes->nodes[last_node], skip_connection);
                            // 3
                            RegexConnection repeat_connection = {
                                .class_str = NULL,
                                .class_len = 0,
                                .next_node = new_last_ref,
                            };
                            pushConnectionToRegexNode(&nodes->nodes[current_node], repeat_connection);
                            current_node = exit_node_ref;
                        } else {
                            // like +
                            RegexNodeRef new_last_ref = pushNodeToRegexNodeSet(nodes, nodes->nodes[last_node]);
                            RegexNode last_node_replacement = REGEX_NODE_INIT;
                            RegexConnection start_connection = {
                                .class_str = NULL,
                                .class_len = 0,
                                .next_node = new_last_ref,
                            };
                            pushConnectionToRegexNode(&last_node_replacement, start_connection);
                            nodes->nodes[last_node] = last_node_replacement;
                            // 2
                            RegexNode exit_node = REGEX_NODE_INIT;
                            RegexNodeRef exit_node_ref = pushNodeToRegexNodeSet(nodes, exit_node);
                            RegexConnection end_connection = {
                                .class_str = NULL,
                                .class_len = 0,
                                .next_node = exit_node_ref,
                            };
                            pushConnectionToRegexNode(&nodes->nodes[current_node], end_connection);
                            // 3
                            RegexConnection repeat_connection = {
                                .class_str = NULL,
                                .class_len = 0,
                                .next_node = new_last_ref,
                            };
                            pushConnectionToRegexNode(&nodes->nodes[current_node], repeat_connection);
                            current_node = exit_node_ref;
                        }
                    } else if(max != min) {
                        if(min != 0) {
                            RegexNodeRef new_end = cloneLastNodes(nodes, last_node, current_node);
                            last_node = current_node;
                            current_node = new_end;
                        }
                        RegexConnection connection = {
                            .class_str = NULL,
                            .class_len = 0,
                            .next_node = current_node,
                        };
                        pushConnectionToRegexNode(&nodes->nodes[last_node], connection);
                        for(int i = min + 1; i < max; i++) {
                            RegexNodeRef new_end = cloneLastNodes(nodes, last_node, current_node);
                            last_node = current_node;
                            current_node = new_end;
                        }
                    }
                }
            }
            break;
        }
        case '\\': {
            RegexNode node = REGEX_NODE_INIT;
            RegexNodeRef node_ref = pushNodeToRegexNodeSet(nodes, node);
            RegexConnection connection = {
                .class_str = regex,
                .class_len = 2,
                .next_node = node_ref,
            };
            pushConnectionToRegexNode(&nodes->nodes[current_node], connection);
            last_node = current_node;
            current_node = node_ref;
            regex += 2;
            size -= 2;
            break;
        }
        default: {
            RegexNode node = REGEX_NODE_INIT;
            RegexNodeRef node_ref = pushNodeToRegexNodeSet(nodes, node);
            RegexConnection connection = {
                .class_str = regex,
                .class_len = 1,
                .next_node = node_ref,
            };
            pushConnectionToRegexNode(&nodes->nodes[current_node], connection);
            last_node = current_node;
            current_node = node_ref;
            regex++;
            size--;
            break;
        }
        }
    }
    *end_pos = regex;
    return current_node;
}

RegexNodeRef stringToNfa(RegexNodeSet* nodes, RegexNodeRef start, const char* string, int size) {
    RegexNodeRef current_node = start;
    while(size != 0) {
        RegexNode node = REGEX_NODE_INIT;
        RegexNodeRef node_ref = pushNodeToRegexNodeSet(nodes, node);
        RegexConnection connection = {
            .class_str = string,
            .class_len = 1,
            .next_node = node_ref,
        };
        pushConnectionToRegexNode(&nodes->nodes[current_node], connection);
        current_node = node_ref;
        string++;
        size--;
    }
    return current_node;
}
