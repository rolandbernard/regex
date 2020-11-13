
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

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
                .class = NULL,
                .class_len = 0,
                .next_node = new_start_ref,
            };
            pushConnectionToRegexNode(&start_replacement, start_connection);
            nodes->nodes[start] = start_replacement;
            // 2
            RegexConnection end_connection = {
                .class = NULL,
                .class_len = 0,
                .next_node = exit_node_ref,
            };
            pushConnectionToRegexNode(&nodes->nodes[current_node], end_connection);
            while(*regex == '|') {
                // 3
                RegexNodeRef or_start_node_ref = pushNodeToRegexNodeSet(nodes, exit_node);
                RegexConnection connection = {
                    .class = NULL,
                    .class_len = 0,
                    .next_node = or_start_node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[start], connection);
                regex++;
                RegexNodeRef end_node = parseRegexGroup(nodes, or_start_node_ref, regex, &regex, true);
                if(end_node == -1) {
                    return -1;
                } else {
                    // 4
                    RegexConnection connection = {
                        .class = NULL,
                        .class_len = 0,
                        .next_node = exit_node_ref,
                    };
                    pushConnectionToRegexNode(&nodes->nodes[end_node], connection);
                }
            }
            if(*regex != 0 && *regex != ')') {
                return -1;
            } else {
                last_node = start;
                current_node = exit_node_ref;
            }
            break;
        }
        case '?': {
            if(last_node == -1) {
                return -1;
            } else {
                regex++;
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
                    .class = NULL,
                    .class_len = 0,
                    .next_node = new_last_ref,
                };
                pushConnectionToRegexNode(&last_node_replacement, start_connection);
                nodes->nodes[last_node] = last_node_replacement;
                // 2
                RegexConnection skip_connection = {
                    .class = NULL,
                    .class_len = 0,
                    .next_node = current_node,
                };
                pushConnectionToRegexNode(&nodes->nodes[last_node], skip_connection);
            }
            break;
        }
        case '*': {
            if(last_node == -1) {
                return -1;
            } else {
                regex++;
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
                    .class = NULL,
                    .class_len = 0,
                    .next_node = new_last_ref,
                };
                pushConnectionToRegexNode(&last_node_replacement, start_connection);
                nodes->nodes[last_node] = last_node_replacement;
                // 2
                RegexNode exit_node = REGEX_NODE_INIT;
                RegexNodeRef exit_node_ref = pushNodeToRegexNodeSet(nodes, exit_node);
                RegexConnection end_connection = {
                    .class = NULL,
                    .class_len = 0,
                    .next_node = exit_node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], end_connection);
                // 3
                RegexConnection skip_connection = {
                    .class = NULL,
                    .class_len = 0,
                    .next_node = exit_node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[last_node], skip_connection);
                // 4
                RegexConnection repeat_connection = {
                    .class = NULL,
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
                return -1;
            } else {
                regex++;
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
                    .class = NULL,
                    .class_len = 0,
                    .next_node = new_last_ref,
                };
                pushConnectionToRegexNode(&last_node_replacement, start_connection);
                nodes->nodes[last_node] = last_node_replacement;
                // 2
                RegexNode exit_node = REGEX_NODE_INIT;
                RegexNodeRef exit_node_ref = pushNodeToRegexNodeSet(nodes, exit_node);
                RegexConnection end_connection = {
                    .class = NULL,
                    .class_len = 0,
                    .next_node = exit_node_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], end_connection);
                // 3
                RegexConnection repeat_connection = {
                    .class = NULL,
                    .class_len = 0,
                    .next_node = new_last_ref,
                };
                pushConnectionToRegexNode(&nodes->nodes[current_node], repeat_connection);
                current_node = exit_node_ref;
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
                        .class = NULL,
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
                        // 2
                        RegexNode exit_node = REGEX_NODE_INIT;
                        RegexNodeRef exit_node_ref = pushNodeToRegexNodeSet(nodes, exit_node);
                        RegexConnection end_connection = {
                            .class = NULL,
                            .class_len = 0,
                            .next_node = exit_node_ref,
                        };
                        pushConnectionToRegexNode(&nodes->nodes[current_node], end_connection);
                        // 3
                        RegexConnection repeat_connection = {
                            .class = NULL,
                            .class_len = 0,
                            .next_node = last_node,
                        };
                        pushConnectionToRegexNode(&nodes->nodes[current_node], repeat_connection);
                        current_node = exit_node_ref;
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
        if(class[0] == '.') {
            for(int c = 0; c < 256; c++) {
                output[c] = true;
            }
            output['\n'] = false;
        } else {
            for(int c = 0; c < 256; c++) {
                output[c] = false;
            }
            if(class[0] == '$') {
                output['\n'] = true;
            } else {
                output[(unsigned char)class[0]] = true;
            }
        }
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

typedef struct {
    RegexNodeRef* nodes;
    int num_nodes;
    int id;
} RegexNodeCollection;

typedef struct {
    RegexNodeCollection* nodes;
    int node_count;
    int node_capacity;
    int queue_start;
    int queue_end;
} RegexNodeCollectionQueue;
#define REGEX_NODE_COLLECTION_QUEUE_INIT { .nodes = NULL, .node_count = 0, .node_capacity = 0, .queue_start = 0, .queue_end = 0 }

static void pushToNodeQueue(RegexNodeCollectionQueue* queue, RegexNodeCollection col) {
    if(queue->node_capacity == queue->node_count) {
        if(queue->node_capacity == 0) {
            queue->node_capacity = 4;
            queue->nodes = (RegexNodeCollection*)malloc(sizeof(RegexNodeCollection) * queue->node_capacity);
        } else {
            int new_capacity = 2 * queue->node_capacity;
            queue->nodes = (RegexNodeCollection*)realloc(queue->nodes, sizeof(RegexNodeCollection) * new_capacity);
            memmove(queue->nodes + new_capacity - queue->node_capacity + queue->queue_start,
                queue->nodes + queue->queue_start, (queue->node_capacity - queue->queue_start) * sizeof(RegexNodeCollection));
            queue->queue_start = new_capacity - queue->node_capacity + queue->queue_start;
            queue->node_capacity = new_capacity;
        }
    }
    queue->nodes[queue->queue_end] = col;
    queue->queue_end = (queue->queue_end + 1) % queue->node_capacity;
    queue->node_count++;
}

static RegexNodeCollection popFromNodeQueue(RegexNodeCollectionQueue* queue) {
    RegexNodeCollection ret = queue->nodes[queue->queue_start];
    queue->queue_start = (queue->queue_start + 1) % queue->node_capacity;
    queue->node_count--;
    return ret;
}

static bool isContainedInQueue(RegexNodeCollectionQueue* queue, RegexNodeCollection col, int* id) {
    for(int i = queue->queue_start; i != queue->queue_end; i = (i + 1) % queue->node_capacity) {
        if(queue->nodes[i].num_nodes == col.num_nodes) {
            bool equal = true;
            for(int j = 0; equal && j < col.num_nodes; j++) {
                if(queue->nodes[i].nodes[j] != col.nodes[j]) {
                    equal = false;
                }
            }
            if(equal) {
                *id = queue->nodes[i].id;
                return true;
            }
        }
    }
    return false;
}

static int recursiveNodeReachabilitySearch(RegexNodeSet* nodes, RegexNodeRef node, bool* visited) {
    if(!visited[node]) {
        visited[node] = true;
        int ret = 1;
        for(int i = 0; i < nodes->nodes[node].connection_count; i++) {
            RegexConnection connection = nodes->nodes[node].connections[i];
            if(connection.class == NULL) {
                ret += recursiveNodeReachabilitySearch(nodes, connection.next_node, visited);
            }
        }
        return ret;
    } else {
        return 0;
    }
}

static RegexNodeCollection getNodesDirectlyReachableFrom(RegexNodeSet* nodes, RegexNodeRef from) {
    bool nodes_reachable[nodes->node_count];
    int num_nodes = recursiveNodeReachabilitySearch(nodes, from, nodes_reachable);
    RegexNodeCollection ret = {
        .nodes = (RegexNodeRef*)malloc(sizeof(RegexNodeRef) * num_nodes),
        .num_nodes = num_nodes,
        .id = 0,
    };
    for(int i = 0, j = 0; i < nodes->node_count; i++) {
        if(nodes_reachable[i]) {
            ret.nodes[j] = i;
            j++;
        }
    }
    return ret;
}

typedef struct {
    RegexStateTransition (*regex)[256];
    int count;
    int capacity;
} RegexStateList;
#define REGEX_STATE_LIST_INIT { .regex = NULL, .count = 0, .capacity = 0 }

static void pushToRegexStateList(RegexStateList* list, RegexStateTransition val[256]) {
    if(list->capacity == list->count) {
        if(list->capacity == 0) {
            list->capacity = 4;
        } else {
            list->capacity *= 2;
        }
        list->regex = (RegexStateTransition (*)[256])realloc(list->regex, sizeof(RegexStateTransition[256]) * list->capacity);
    }
    memcpy(list->regex[list->count], val, 256 * sizeof(RegexStateTransition));
    list->count++;
}

static Regex compileRegexToStateMachine(RegexNodeSet* nodes, RegexNodeRef start) {
    RegexNodeCollectionQueue to_resolve = REGEX_NODE_COLLECTION_QUEUE_INIT;
    RegexNodeCollectionQueue resolved = REGEX_NODE_COLLECTION_QUEUE_INIT;
    pushToNodeQueue(&to_resolve, getNodesDirectlyReachableFrom(nodes, start));
    RegexStateList ret = REGEX_STATE_LIST_INIT;
    int total_count = 1;
    while(to_resolve.node_count > 0) {
        RegexNodeCollection col = popFromNodeQueue(&to_resolve);

    }
    return ret.regex;
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
        Regex ret = compileRegexToStateMachine(&nodes, start_ref);
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
