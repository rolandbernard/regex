#ifndef _REGEX_H_
#define _REGEX_H_

#include <stdbool.h>

#include "regex-type.h"

int getRegexErrorLocationN(const char* regex_string, int len);

int getRegexErrorLocation(const char* regex_string);

Regex compileMatchingRegexN(const char* regex_string, int len);

Regex compileMatchingRegex(const char* regex_string);

Regex compileMatchingStringN(const char* string, int len);

Regex compileMatchingString(const char* string);

Regex compileMultiMatchingRegexN(int num_regex, const char* const* regex_strings, const int* lengths);

Regex compileMultiMatchingRegex(int num_regex, const char* const* regex_strings);

Regex compileMultiMatchingStringsN(int num_strings, const char* const* strings, const int* lengths);

Regex compileMultiMatchingStrings(int num_strings, const char* const* strings);

Regex compileMultiMatchingStringsAndRegexN(int num_patterns, const bool* is_regex, const char* const* patterns, const int* lengths);

Regex compileMultiMatchingStringsAndRegex(int num_patterns, const bool* is_regex, const char* const* patterns);

bool startsWithRegexN(Regex regex, const char* string, int size, int* len_out, int* exit_num);

bool startsWithRegex(Regex regex, const char* string, int* len, int* exit_num);

bool matchRegex(Regex regex, const char* string, int* exit_num);

bool matchRegexN(Regex regex, const char* string, int size, int* exit_num);

void disposeRegex(Regex regex);

void printRegexDfa(Regex reg);

#endif