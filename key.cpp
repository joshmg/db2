// File: key.cpp
// Written by Joshua Green


#include "key.h"
#include "column.h"
#include "query.h"
#include "str.h"

#include <string>
#include <iostream>
using namespace std;

key_entry::key_entry() {
  col = 0;
  fpos = 0;
}

key_entry::key_entry(column *_key, std::string _data, long long int _fpos, long int _id) {
  col = _key;
  data = _data;
  fpos = _fpos;
  row_id = _id;
}

key_entry::~key_entry() {}

bool key_entry::operator==(const key_entry &comp) {
  return (data == comp.data && (*col) == (*comp.col));
}
