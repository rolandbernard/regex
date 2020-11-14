#ifndef _REGEX_DFA_H_
#define _REGEX_DFA_H_

#include "regex-nfa.h"
#include "regex-type.h"

Regex compileRegexToStateMachine(RegexNodeSet* nodes, RegexNodeRef start);

#endif