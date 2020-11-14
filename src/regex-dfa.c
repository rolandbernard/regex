
#include <ctype.h>
#include <string.h>

#include "regex-dfa.h"

static void resolveCharacterClass(const char* class_str, int len, bool output[256]) {
    if (len == 1) {
        if(class_str[0] == '.') {
            for(int c = 0; c < 256; c++) {
                output[c] = true;
            }
            output['\n'] = false;
        } else {
            for(int c = 0; c < 256; c++) {
                output[c] = false;
            }
            if(class_str[0] == '$') {
                output['\n'] = true;
            } else {
                output[(unsigned char)class_str[0]] = true;
            }
        }
    } else if (class_str[0] == '[') {
        bool inverted = false;
        int i = 1;
        if(class_str[i] == '^') {
            inverted = true;
            i++;
        }
        for(int c = 0; c < 256; c++) {
            output[c] = inverted;
        }
        for(; i < len - 1; i++) {
            char c = class_str[i];
            if(c == '\\') {
                i++;
                switch (class_str[i]) {
                case 'w':
                    for (int c = 0; c < 256; c++) {
                        if(isalnum(c) || c == '_') {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'W':
                    for (int c = 0; c < 256; c++) {
                        if(!(isalnum(c) || c == '_')) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'd':
                    for (int c = 0; c < 256; c++) {
                        if(isdigit(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'D':
                    for (int c = 0; c < 256; c++) {
                        if(!isdigit(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 's':
                    for (int c = 0; c < 256; c++) {
                        if(isspace(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'S':
                    for (int c = 0; c < 256; c++) {
                        if(!isspace(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'p':
                    for (int c = 0; c < 256; c++) {
                        if(isprint(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'P':
                    for (int c = 0; c < 256; c++) {
                        if(!isprint(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'g':
                    for (int c = 0; c < 256; c++) {
                        if(isgraph(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'G':
                    for (int c = 0; c < 256; c++) {
                        if(!isgraph(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'l':
                    for (int c = 0; c < 256; c++) {
                        if(islower(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'L':
                    for (int c = 0; c < 256; c++) {
                        if(!islower(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'u':
                    for (int c = 0; c < 256; c++) {
                        if(isupper(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'U':
                    for (int c = 0; c < 256; c++) {
                        if(!isupper(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'x':
                    for (int c = 0; c < 256; c++) {
                        if(isxdigit(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'X':
                    for (int c = 0; c < 256; c++) {
                        if(!isxdigit(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'a':
                    for (int c = 0; c < 256; c++) {
                        if(isalpha(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                case 'A':
                    for (int c = 0; c < 256; c++) {
                        if(!isalpha(c)) {
                            output[c] = !inverted;
                        }
                    }
                    break;
                default: {
                    switch (class_str[i]) {
                    case 'n':
                        c = '\n';
                        break;
                    case 'r':
                        c = '\r';
                        break;
                    case 't':
                        c = '\t';
                        break;
                    case 'e':
                        c = '\e';
                        break;
                    // case 'a': // This will not work because \a matches an alphabetic character
                    //     c = '\a';
                    //     break;
                    case 'b':
                        c = '\b';
                        break;
                    case 'v':
                        c = '\v';
                        break;
                    case 'f':
                        c = '\f';
                        break;
                    case '0':
                        c = '\0';
                        break;
                    default: {
                        c = class_str[i];
                        break;
                    }
                    }
                    break;
                }
                }
                if(c == '\\') {
                    continue;
                }
            }
            if(class_str[i + 1] == '-' && i + 2 < len - 1) {
                i += 2;
                char end = class_str[i];
                if(end == '\\') {
                    i++;
                    switch (class_str[i]) {
                    case 'n':
                        c = '\n';
                        break;
                    case 'r':
                        c = '\r';
                        break;
                    case 't':
                        c = '\t';
                        break;
                    case 'e':
                        c = '\e';
                        break;
                    case 'a': 
                        c = '\a';
                        break;
                    case 'b':
                        c = '\b';
                        break;
                    case 'v':
                        c = '\v';
                        break;
                    case 'f':
                        c = '\f';
                        break;
                    case '0':
                        c = '\0';
                        break;
                    default: {
                        c = class_str[i];
                        break;
                    }
                    }
                }
                for(int j = c; j <= end; j++) {
                    output[j] = !inverted;
                }
            } else {
                output[(unsigned char)c] = !inverted;
            }
        }
    } else if (class_str[0] == '\\') {
        switch (class_str[1]) {
        case 'w':
            for(int c = 0; c < 256; c++) {
                output[c] = isalnum(c) || c == '_';
            }
            break;
        case 'W':
            for(int c = 0; c < 256; c++) {
                output[c] = !(isalnum(c) || c == '_');
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
        case 'p':
            for(int c = 0; c < 256; c++) {
                output[c] = isprint(c);
            }
            break;
        case 'P':
            for(int c = 0; c < 256; c++) {
                output[c] = !isprint(c);
            }
            break;
        case 'g':
            for(int c = 0; c < 256; c++) {
                output[c] = isgraph(c);
            }
            break;
        case 'G':
            for(int c = 0; c < 256; c++) {
                output[c] = !isgraph(c);
            }
            break;
        case 'l':
            for(int c = 0; c < 256; c++) {
                output[c] = islower(c);
            }
            break;
        case 'L':
            for(int c = 0; c < 256; c++) {
                output[c] = !islower(c);
            }
            break;
        case 'u':
            for(int c = 0; c < 256; c++) {
                output[c] = isupper(c);
            }
            break;
        case 'U':
            for(int c = 0; c < 256; c++) {
                output[c] = !isupper(c);
            }
            break;
        case 'x':
            for(int c = 0; c < 256; c++) {
                output[c] = isxdigit(c);
            }
            break;
        case 'X':
            for(int c = 0; c < 256; c++) {
                output[c] = !isxdigit(c);
            }
            break;
        case 'a':
            for(int c = 0; c < 256; c++) {
                output[c] = isalpha(c);
            }
            break;
        case 'A':
            for(int c = 0; c < 256; c++) {
                output[c] = !isalpha(c);
            }
            break;
        default: {
            for(int c = 0; c < 256; c++) {
                output[c] = false;
            }
            switch (class_str[1]) {
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
                // case 'a': // This will not work because \a matches an alphabetic character
                //     output['\a'] = true;
                //     break;
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
                    output[(unsigned char)class_str[1]] = true;
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
    int exit_num;
    int id;
} RegexNodeCollection;

typedef struct {
    RegexNodeCollection* nodes;
    int node_count;
    int node_capacity;
    int queue_start;
} RegexNodeCollectionQueue;
#define REGEX_NODE_COLLECTION_QUEUE_INIT { .nodes = NULL, .node_count = 0, .node_capacity = 0, .queue_start = 0 }

static void pushToNodeQueue(RegexNodeCollectionQueue* queue, RegexNodeCollection col) {
    if(queue->node_capacity == queue->node_count) {
        if(queue->node_capacity == 0) {
            queue->node_capacity = 16;
        } else {
            queue->node_capacity *= 2;
        }
        queue->nodes = (RegexNodeCollection*)realloc(queue->nodes, sizeof(RegexNodeCollection) * queue->node_capacity);
    }
    queue->nodes[queue->node_count] = col;
    queue->node_count++;
}

static RegexNodeCollection popFromNodeQueue(RegexNodeCollectionQueue* queue) {
    RegexNodeCollection ret = queue->nodes[queue->queue_start];
    queue->queue_start++;
    return ret;
}

static bool isContainedInQueue(RegexNodeCollectionQueue* queue, RegexNodeCollection col, int* id) {
    for(int i = 0; i != queue->node_count; i++) {
        if(queue->nodes[i].num_nodes == col.num_nodes) {
            bool equal = true;
            for(int j = 0; equal && j < col.num_nodes; j++) {
                if(queue->nodes[i].nodes[j] != col.nodes[j]) {
                    equal = false;
                }
            }
            if(equal && queue->nodes[i].exit_num == col.exit_num) {
                *id = queue->nodes[i].id;
                return true;
            }
        }
    }
    return false;
}

static int recursiveNodeReachabilitySearch(RegexNodeSet* nodes, RegexNodeRef node, bool* visited, bool* to_add) {
    if(!visited[node]) {
        visited[node] = true;
        int ret = 0;
        bool has_non_null = false;
        for(int i = 0; i < nodes->nodes[node].connection_count; i++) {
            RegexConnection connection = nodes->nodes[node].connections[i];
            if(connection.class_str == NULL) {
                ret += recursiveNodeReachabilitySearch(nodes, connection.next_node, visited, to_add);
            } else {
                has_non_null = true;
            }
        }
        if(has_non_null) {
            to_add[node] = true;
            return ret + 1;
        } else {
            return ret;
        }
    } else {
        return 0;
    }
}

static RegexNodeCollection getNodesDirectlyReachableFrom(RegexNodeSet* nodes, RegexNodeRef* from, int num_from) {
    bool visited[nodes->node_count];
    bool nodes_reachable[nodes->node_count];
    memset(nodes_reachable, 0, sizeof(nodes_reachable));
    memset(visited, 0, sizeof(visited));
    int num_nodes = 0;
    for(int i = 0; i < num_from; i++) {
        num_nodes += recursiveNodeReachabilitySearch(nodes, from[i], visited, nodes_reachable);
    }
    RegexNodeCollection ret = {
        .nodes = (RegexNodeRef*)malloc(sizeof(RegexNodeRef) * num_nodes),
        .num_nodes = num_nodes,
        .id = 0,
        .exit_num = -1,
    };
    for(int i = 0, j = 0; i < nodes->node_count; i++) {
        if(nodes_reachable[i]) {
            ret.nodes[j] = i;
            j++;
        }
        if(visited[i] && nodes->nodes[i].exit_num != -1) {
            ret.exit_num = nodes->nodes[i].exit_num;
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

static void pushToRegexStateList(RegexStateList* list) {
    if(list->capacity == list->count) {
        if(list->capacity == 0) {
            list->capacity = 16;
        } else {
            list->capacity *= 2;
        }
        list->regex = (RegexStateTransition (*)[256])realloc(list->regex, sizeof(RegexStateTransition[256]) * list->capacity);
    }
    memset(list->regex[list->count], 0, 256 * sizeof(RegexStateTransition));
    list->count++;
}

static bool areLinesEqual(bool (*data)[256], int line1, int line2, int row_len) {
    for(int i = 0; i < row_len; i++) {
        if(data[i][line1] != data[i][line2]) {
            return false;
        }
    }
    return true;
}

Regex compileRegexToStateMachine(RegexNodeSet* nodes, RegexNodeRef start) {
    RegexStateList ret = REGEX_STATE_LIST_INIT;
    RegexNodeCollectionQueue to_resolve = REGEX_NODE_COLLECTION_QUEUE_INIT;
    pushToNodeQueue(&to_resolve, getNodesDirectlyReachableFrom(nodes, &start, 1));
    pushToRegexStateList(&ret);
    while(to_resolve.node_count > to_resolve.queue_start) {
        RegexNodeCollection col = popFromNodeQueue(&to_resolve);
        int num_connections = 0;
        for(int n = 0; n < col.num_nodes; n++) {
            RegexNode node = nodes->nodes[col.nodes[n]];
            for(int c = 0; c < node.connection_count; c++) {
                if(node.connections[c].class_str != NULL) {
                    num_connections++;
                }
            }
        }
        if(num_connections != 0) {
            int connection_dests[num_connections];
            bool connection_class[num_connections][256];
            int index = 0;
            for (int n = 0; n < col.num_nodes; n++) {
                RegexNode node = nodes->nodes[col.nodes[n]];
                for (int c = 0; c < node.connection_count; c++) {
                    if (node.connections[c].class_str != NULL) {
                        resolveCharacterClass(node.connections[c].class_str, node.connections[c].class_len, connection_class[index]);
                        connection_dests[index] = node.connections[c].next_node;
                        index++;
                    }
                }
            }
            int start_index = 0;
            int end_index = 1;
            while (start_index < 256) {
                if (end_index < 256 && areLinesEqual(connection_class, start_index, end_index, num_connections)) {
                    end_index++;
                } else {
                    int num_dests = 0;
                    for (int i = 0; i < num_connections; i++) {
                        if (connection_class[i][start_index]) {
                            num_dests++;
                        }
                    }
                    if (num_dests > 0) {
                        RegexNodeRef dests[num_dests];
                        index = 0;
                        for (int i = 0; i < num_connections; i++) {
                            if (connection_class[i][start_index]) {
                                dests[index] = connection_dests[i];
                                index++;
                            }
                        }
                        RegexNodeCollection dest_nodes = getNodesDirectlyReachableFrom(nodes, dests, num_dests);
                        int next_node;
                        if (!isContainedInQueue(&to_resolve, dest_nodes, &next_node)) {
                            dest_nodes.id = ret.count;
                            next_node = dest_nodes.id;
                            pushToRegexStateList(&ret);
                            pushToNodeQueue(&to_resolve, dest_nodes);
                        } else {
                            free(dest_nodes.nodes);
                        }
                        for (int i = start_index; i < end_index; i++) {
                            ret.regex[col.id][i].state_type = REGEX_STATE_NEXT;
                            ret.regex[col.id][i].next_state = next_node;
                        }
                    } else if (col.exit_num != -1) {
                        for (int i = start_index; i < end_index; i++) {
                            ret.regex[col.id][i].state_type = REGEX_STATE_END;
                            ret.regex[col.id][i].end_point = col.exit_num;
                        }
                    }
                    start_index = end_index;
                    end_index = start_index + 1;
                }
            }
        } else if(col.exit_num != -1) {
            for (int i = 0; i < 256; i++) {
                ret.regex[col.id][i].state_type = REGEX_STATE_END;
                ret.regex[col.id][i].end_point = col.exit_num;
            }
        }
    }
    for(int i = 0; i < to_resolve.node_count; i++) {
        free(to_resolve.nodes[i].nodes);
    }
    free(to_resolve.nodes);
    return (Regex)realloc(ret.regex, sizeof(RegexStateTransition[256]) * ret.count);
}
