
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
            node->connection_capacity = 4;
        } else {
            node->connection_capacity *= 2;
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

static RegexNodeRef cloneLastNodes(RegexNodeSet* nodes, RegexNodeRef start, RegexNodeRef end) {
    RegexNodeRef last_node = nodes->node_count - 1;
    for(int n = start; n < last_node; n++) {
        RegexNode node = REGEX_NODE_INIT;
        pushNodeToRegexNodeSet(nodes, node);
    }
    for(int n = start; n <= last_node; n++) {
        for(int c = 0; c < nodes->nodes[n].connection_count; c++) {
            RegexConnection connection = nodes->nodes[n].connections[c];
            if(connection.next_node >= start && connection.next_node <= last_node) {
                connection.next_node += last_node - start;
                pushConnectionToRegexNode(&nodes->nodes[n + (last_node - start)], connection);
            }
        }
    }
    return end + (last_node - start);
}

static RegexNodeRef parseRegexGroup(RegexNodeSet* nodes, RegexNodeRef start, const char* regex, const char** end_pos, bool inside_or) {
    RegexNodeRef last_node = -1;
    RegexNodeRef current_node = start;
    while(*regex != 0 && *regex != ')' && (!inside_or || *regex != '|')) {
        switch (*regex) {
        case '[': {
            int len = 1;
            while(regex[len] != 0 && regex[len] != ']') {
                if(regex[len] == '\\') {
                    len++;
                }
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
            } else {
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
                    for(int i = 1; i < min; i++) {
                        RegexNodeRef new_end = cloneLastNodes(nodes, last_node, current_node);
                        last_node = current_node;
                        current_node = new_end;
                    }
                    if(max == -1) {
                        RegexConnection connection_repeat = {
                            .class = NULL,
                            .class_len = 0,
                            .next_node = last_node,
                        };
                        pushConnectionToRegexNode(&nodes->nodes[current_node], connection_repeat);
                    } else if(max != min) {
                        RegexNodeRef new_end = cloneLastNodes(nodes, last_node, current_node);
                        last_node = current_node;
                        current_node = new_end;
                        RegexConnection connection = {
                            .class = NULL,
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

static void resolveCharacterClass(const char* class, int len, bool* output) {
    if (len == 1) {
        for(int c = 0; c < 256; c++) {
            output[c] = false;
        }
        output[(unsigned char)class[0]] = true;
    } else if (class[0] == '[') {
        bool inverted = false;
        int i = 1;
        if(class[i] == '^') {
            inverted = true;
            i++;
        }
        for(int c = 0; c < 256; c++) {
            output[c] = inverted;
        }
        for(; i < len - 1; i++) {
            char c = class[i];
            if(c == '\\') {
                i++;
                c = class[i];
            }
            if(class[i] == '-') {
                i++;
                char end = class[i];
                for(int j = c; j <= end; j++) {
                    output[j] = !inverted;
                }
            } else {
                output[(unsigned char)c] = !inverted;
            }
        }
    } else if (class[0] == '\\') {
        switch (class[1]) {
        case 'w':
            for(int c = 0; c < 256; c++) {
                output[c] = isalnum(c);
            }
            break;
        case 'W':
            for(int c = 0; c < 256; c++) {
                output[c] = !isalnum(c);
            }
            break;
        case 'd':
            for(int c = 0; c < 256; c++) {
                output[c] = isdigit(c);
            }
            break;
        case 'D':
            for(int c = 0; c < 256; c++) {
                output[c] = !isdigit(c);
            }
            break;
        case 's':
            for(int c = 0; c < 256; c++) {
                output[c] = isspace(c);
            }
            break;
        case 'S':
            for(int c = 0; c < 256; c++) {
                output[c] = !isspace(c);
            }
            break;
        default: {
            for(int c = 0; c < 256; c++) {
                output[c] = false;
            }
            switch (class[1]) {
                case 'n':
                    output['\n'] = true;
                    break;
                case 'r':
                    output['\r'] = true;
                    break;
                case 't':
                    output['\t'] = true;
                    break;
                case 'e':
                    output['\e'] = true;
                    break;
                case 'a':
                    output['\a'] = true;
                    break;
                case 'b':
                    output['\b'] = true;
                    break;
                case 'v':
                    output['\v'] = true;
                    break;
                case 'f':
                    output['\f'] = true;
                    break;
                case '0':
                    output['\0'] = true;
                    break;
                default: {
                    output[(unsigned char)class[1]] = true;
                    break;
                }
            }
            break;
        }
        }
    }
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
    if(last_node < 0 || *end_pos != 0) {
        freeNodes(&nodes);
        return NULL;
    } else {
        nodes.nodes[last_node].exit_num = 0;
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
