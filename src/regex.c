
#include <stdlib.h>
#include <ctype.h>

#include "regex.h"

typedef int RegexNodeRef;

typedef struct {
    const char* class;
    int class_len;
    RegexNodeRef next_node;
} RegexConnection;

typedef struct {
    RegexConnection* connections;
    int connection_count;
    int connection_capacity;
    int exit_num;
} RegexNode;
#define REGEX_NODE_INIT { .connections = NULL, .connection_count = 0, .connection_capacity = 0, .exit_num = -1 }

typedef struct {
    RegexNode* nodes;
    int node_count;
    int node_capacity;
} RegexNodeSet;
#define REGEX_NODE_SET_INIT { .nodes = NULL, .node_count = 0, .node_capacity = 0 }

static void pushConnectionToRegexNode(RegexNode* node, RegexConnection conn) {
    if(node->connection_capacity == node->connection_count) {
        if(node->connection_capacity == 0) {
            node->connection_count = 4;
        } else {
            node->connection_count *= 2;
        }
        node->connections = (RegexConnection*)realloc(node->connections, sizeof(RegexConnection) * node->connection_capacity);
    }
    node->connections[node->connection_count] = conn;
    node->connection_count++;
}

static RegexNodeRef pushNodeToRegexNodeSet(RegexNodeSet* node_set, RegexNode node) {
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

static void freeNodes(RegexNodeSet* node_set) {
    for(int n = 0; n < node_set->node_count; n++) {
        free(node_set->nodes[n].connections);
    }
    free(node_set->nodes);
}

static RegexNodeRef copyNodesBetween(RegexNodeSet* nodes, RegexNodeRef start, RegexNodeRef end, RegexNodeRef destination) {
    return -1;
}

static RegexNodeRef parseRegexGroup(RegexNodeSet* nodes, RegexNodeRef start, const char* regex, const char** end_pos, bool inside_or) {
    RegexNodeRef last_node = -1;
    RegexNodeRef current_node = start;
    while(*regex != 0 && *regex != ')' && (!inside_or || *regex != '|')) {
        switch (*regex) {
        case '[': {
            int len = 1;
            while(regex[len] != 0 && regex[len] != ']') {
                len++;
            }
            if(regex[len] == 0) {
                return -1;
            } else {
                len++;
                RegexNode node = REGEX_NODE_INIT;
                RegexNodeRef node_ref = pushNodeToRegexNodeSet(nodes, node);
                RegexConnection connection = {
                    .class = regex,
                    .class_len = len,
                    .next_node = node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], connection);
                last_node = current_node;
                current_node = node_ref;
                regex += len;
            }
            break;
        }
        case '(': {
            regex++;
            last_node = current_node;
            current_node = parseRegexGroup(nodes, current_node, regex, &regex, false);
            if(current_node == -1 || *regex != ')') {
                return -1;
            } else {
                regex++;
            }
            break;
        }
        case '|': {
            RegexNode node = REGEX_NODE_INIT;
            RegexNodeRef node_ref = pushNodeToRegexNodeSet(nodes, node);
            while(*regex == '|') {
                regex++;
                RegexNodeRef end_node = parseRegexGroup(nodes, start, regex, &regex, true);
                if(end_node == -1) {
                    return -1;
                } else {
                    RegexConnection connection = {
                        .class = NULL,
                        .class_len = 0,
                        .next_node = node_ref,
                    };
                    pushConnectionToRegexNode(&nodes->nodes[end_node], connection);
                }
            }
            if(*regex != 0 && *regex != ')') {
                return -1;
            } else {
                last_node = current_node;
                current_node = node_ref;
            }
            break;
        }
        case '?': {
            if(last_node == -1) {
                return -1;
            } else {
                RegexConnection connection = {
                    .class = NULL,
                    .class_len = 0,
                    .next_node = current_node,
                };
                pushConnectionToRegexNode(&nodes->nodes[last_node], connection);
            }
            break;
        }
        case '*': {
            if(last_node == -1) {
                return -1;
            } else {
                RegexConnection connection_skip = {
                    .class = NULL,
                    .class_len = 0,
                    .next_node = current_node,
                };
                pushConnectionToRegexNode(&nodes->nodes[last_node], connection_skip);
                RegexConnection connection_repeat = {
                    .class = NULL,
                    .class_len = 0,
                    .next_node = last_node,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], connection_repeat);
            }
            break;
        }
        case '+': {
            if(last_node == -1) {
                return -1;
            } else {
                RegexConnection connection_repeat = {
                    .class = NULL,
                    .class_len = 0,
                    .next_node = last_node,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], connection_repeat);
            }
            break;
        }
        case '{': {
            if(last_node == -1) {
                return -1;
            }
            regex++;
            int min = 0;
            while (isspace(*regex)) {
                regex++;
            }
            while (*regex >= '0' && *regex <= '9') {
                min *= 10;
                min += *regex - '0';
                regex++;
            }
            while (isspace(*regex)) {
                regex++;
            }
            int max = min;
            if(*regex == ',') {
                while (isspace(*regex)) {
                    regex++;
                }
                regex++;
                if(*regex == '}') {
                    max = -1;
                } else {
                    max = 0;
                    while (*regex >= '0' && *regex <= '9') {
                        max *= 10;
                        max += *regex - '0';
                        regex++;
                    }
                }
                while (isspace(*regex)) {
                    regex++;
                }
            }
            if(*regex != '}' || (max != -1 && max < min)) {
                return -1;
            } else {
                regex++;
            }
            
            break;
        }
        case '\\': {
            switch (regex[1]) {
            case 'd':
            case 'D':
            case 'w':
            case 'W':
            case 's':
            case 'S': {
                RegexNode node = REGEX_NODE_INIT;
                RegexNodeRef node_ref = pushNodeToRegexNodeSet(nodes, node);
                RegexConnection connection = {
                    .class = regex,
                    .class_len = 2,
                    .next_node = node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], connection);
                last_node = current_node;
                current_node = node_ref;
                regex += 2;
                break;
            }
            default: {
                regex++;
                RegexNode node = REGEX_NODE_INIT;
                RegexNodeRef node_ref = pushNodeToRegexNodeSet(nodes, node);
                RegexConnection connection = {
                    .class = regex,
                    .class_len = 1,
                    .next_node = node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], connection);
                last_node = current_node;
                current_node = node_ref;
                regex++;
                break;
            }
            }
            break;
        }
        default: {
            RegexNode node = REGEX_NODE_INIT;
            RegexNodeRef node_ref = pushNodeToRegexNodeSet(nodes, node);
            RegexConnection connection = {
                .class = regex,
                .class_len = 1,
                .next_node = node_ref,
            };
            pushConnectionToRegexNode(&nodes->nodes[current_node], connection);
            last_node = current_node;
            current_node = node_ref;
            regex++;
            break;
        }
        }
    }
    *end_pos = regex;
    return current_node;
}

static Regex compileRegexToStateMashine(RegexNodeSet* nodes, RegexNodeRef start) {
    return NULL;
}

Regex compileMatchingRegex(const char* regex_string) {
    RegexNodeSet nodes = REGEX_NODE_SET_INIT;
    RegexNode start = REGEX_NODE_INIT;
    RegexNodeRef start_ref = pushNodeToRegexNodeSet(&nodes, start);
    const char* end_pos;
    RegexNodeRef last_node = parseRegexGroup(&nodes, start_ref, regex_string, &end_pos, false);
    nodes.nodes[last_node].exit_num = 0;
    if(last_node < 0 || *end_pos != 0) {
        freeNodes(&nodes);
        return NULL;
    } else {
        Regex ret = compileRegexToStateMashine(&nodes, start_ref);
        freeNodes(&nodes);
        return ret;
    }
}

Regex compileMultiMatchingRegex(int num_regex, const char* const* regex_strings) {
    return NULL;
}

bool startsWithRegex(Regex regex, const char* string, int* len, int* exit_num) {
    return false;
}

bool matchRegex(Regex regex, const char* string, int* exit_num) {
    return false;
}

void disposeRegex(Regex regex) {

}
