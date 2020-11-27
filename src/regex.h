#ifndef _REGEX_H_
#define _REGEX_H_

#include <stdbool.h>

#include "regex-type.h"

Regex compileMatchingRegex(const char* regex_string);

Regex compileMatchingString(const char* string);

Regex compileMultiMatchingRegex(int num_regex, const char* const* regex_strings);

Regex compileMultiMatchingStrings(int num_strings, const char* const* strings);

Regex compileMultiMatchingStringsAndRegex(int num_patterns, const bool* is_regex, const char* const* patterns);

bool startsWithRegex(Regex regex, const char* string, int* len, int* exit_num);

bool matchRegex(Regex regex, const char* string, int* exit_num);

void disposeRegex(Regex regex);

void printRegexDfa(Regex reg);

#endif