// File: str.h
// Written by Joshua Green

#ifndef _H
#define _H

#pragma comment(lib, "str.lib")

#include <string>
#include <vector>

// integer to string
// log time
std::string itos(long int number);

// string to lower
// linear time
std::string strtolower(const std::string&);

// returns true if all of the characters within string are 0-9
// linear time
bool is_numeric(const std::string&);

// string is converted to lowercase, non alpha-numeric values are converted to (char)0
// ie: z > a > 9 > 0 > ('.' == ',' == ';' == '[' == etc...)
bool strlessthan(const std::string&, const std::string&);

// explode(string_to_parse, delim, limit)
// string is parsed into separate strings, removing delim until limit separations has occured
// limit = 0 does not parse the string
// limit < 0 parses the entire string
std::vector<std::string> explode(const std::string&, const std::string&, int);

#endif
