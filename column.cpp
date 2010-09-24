// File: column.cpp
// Written by Joshua Green

#include "column.h"

#include <string>
#include <iostream>
using namespace std;

column::column() {
  size = 0;
  _load_flags("");
}

column::column(const std::string &_name, long int _size, std::string set_flags) {
  name = _name;
  size = _size;
  alias = _name;
  _load_flags(set_flags);
}

column::~column() {}

void column::clear() {
  name.clear();
  alias.clear();
  size = 0;
  _load_flags("");
}

void column::_load_flags(const string& set_flags) {
  for (int i=0;i<26;i++) flags[i] = false;

  for (int i=0;i<set_flags.length();i++) {
    if (set_flags[i] > 96 && set_flags[i] < 123) flags[set_flags[i]-97] = true;
  }
}
  
string column::_encode_flags() {
  string value;
  char c;
  for (int i=0;i<26;i++) {
    c = 96;
    c += flags[i]*(i+1);
    if (c != 96) value += c;
  }
  return value;
}

column &column::auto_inc(bool value) {
  flags[0] = true;
  return (*this);
}
bool column::is_auto_inc() {
  return flags[0];
}

column &column::not_null(bool value) {
  flags[13] = true;
  return (*this);
}
bool column::is_not_null() {
  return flags[13];
}

column &column::set_default(const std::string &value) {
  _default = value;
  return (*this);
}

bool column::operator==(const column &comp) const {
  return (name == comp.name && size == comp.size);
}

bool column::operator==(const column *comp) const {
  return (name == comp->name && size == comp->size);
}

bool column::operator!=(const column &comp) const {
  return (name != comp.name || size != comp.size);
}

bool column::operator!=(const column *comp) const {
  return (name != comp->name || size != comp->size);
}
