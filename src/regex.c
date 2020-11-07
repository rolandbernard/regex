
#include <stdlib.h>

#include "regex.h"

Regex compileMatchingRegex(const char* regex_string) {
    return NULL;
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
