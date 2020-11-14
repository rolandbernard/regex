#ifndef _REGEX_H_
#define _REGEX_H_

#include <stdbool.h>

#include "regex-type.h"

Regex compileMatchingRegex(const char* regex_string);

Regex compileMultiMatchingRegex(int num_regex, const char* const* regex_strings);

bool startsWithRegex(Regex regex, const char* string, int* len, int* exit_num);

bool matchRegex(Regex regex, const char* string, int* exit_num);

void disposeRegex(Regex regex);

#endif