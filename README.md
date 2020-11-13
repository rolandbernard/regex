Regular Expression
==================
A small regular expression library for C.

This regex implementation first compiles the regex into a NFA and then
transforms that NFA into a DFA. The DFA can then be appliend to an input
string of length n in O(n).
