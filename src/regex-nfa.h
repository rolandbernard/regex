#ifndef _REGEX_NFA_H_
#define _REGEX_NFA_H_

#include <stdlib.h>
#include <stdbool.h>

typedef int RegexNodeRef;

typedef struct {
    const char* class_str;
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

void freeNodes(RegexNodeSet* node_set);

RegexNodeRef pushNodeToRegexNodeSet(RegexNodeSet* node_set, RegexNode node);

void pushConnectionToRegexNode(RegexNode* node, RegexConnection conn);

RegexNodeRef parseRegexGroup(RegexNodeSet* nodes, RegexNodeRef start, const char* regex, int len, const char** end_pos, bool inside_or);

RegexNodeRef stringToNfa(RegexNodeSet* nodes, RegexNodeRef start, const char* string, int len);

#endif
