// File: row.cpp
// Written by Joshua Green

#include "row.h"
#include "column.h"
#include "table.h"

#include <string>
#include <map>
#include <iostream>
#include <cmath>
using namespace std;

row::row() {
  id = -1;
}
row::row(int _id) {
  id = _id;
}

row::~row() {}

row &row::add(const string &key, const string &data) {
  _data.insert(_data.end(), pair<string, string>(key, data));
  return (*this);
}

row &row::add(const column &col, const string &data) {
  _data.insert(_data.end(), pair<string, string>(col.name, data));
  return (*this);
}

row &row::remove(const string &key) {
  _data.erase(key);
  return (*this);
}

row &row::remove(const column &col) {
  _data.erase(col.name);
  return (*this);
}

string row::operator[](const column &col) const {
  return (*this)[col.name];
}

string row::operator[](const string& col) const {
  map<string,string>::const_iterator it = _data.find(col);
  if (it == _data.end()) return "";
  else return it->second;
}

void row::clear() {
  _data.clear();
  id = -1;
}

bool row::is_empty() const {
  return (_data.size() == 0);
}

int row::get_id() const {
  return id;
}

bool row::is_defined(const column& col) const {
  return is_defined(col.name);
}

bool row::is_defined(const string& col) const {
  map<string,string>::const_iterator it = _data.find(col);
  if (it == _data.end()) return false;
  else return true;
}

std::string row::to_str() const {
  string value;
  map<string, string>::const_iterator it = _data.begin();
  while (it != _data.end()) {
    value += it->first;
    value += (char)12;
    value += it->second;
    value += (char)12;
    it++;
  }
  return value;
}

row& row::from_str(const std::string& data) {
  // one|two|three|four|
  // 0123456789012345678
  clear();
  int i=0;
  while (i < data.length()) {
    string col = data.substr(i, data.find((char)12, i)-i);
    i += col.length()+1;

    if (i > data.length()) break;

    string value = data.substr(i, data.find((char)12, i)-i);
    i += value.length()+1;

    add(col, value);
  }

  return (*this);
}

#ifndef DB_LITE
void row::print(table* t) const {
  cout << "ROW";
  
  if (t != 0) {
    cout << " [";
    vector<column> Tstruct = t->get_structure();
    // all this work to dynamically calculate the varying digits required to display for row_id
    float ln10 = log(10.0f);
    int digit_count = (int)(log((float)t->size()-1) / ln10);
    if (id > 0) digit_count -= (int)floor((log((float)id) / ln10));
    else if (id != 0) digit_count++;
    
    for (int i=0; i<digit_count; i++) cout << ' ';
    if (id >= 0) cout << id;
    cout << "] (";
  
    for (int i=0;i<Tstruct.size();i++) {
      //for (int j=0;j<Tstruct[i].size-(*this)[Tstruct[i]].length();j++) cout << ' ';
      cout << (*this)[Tstruct[i]];
      if (i < Tstruct.size()-1) cout << ", ";
    }
    cout << ")" << endl;
  }
  else {
    cout << ":\t (";
    for (map<string,string>::const_iterator it = _data.begin();it != _data.end();) {
      cout << it->second;
      it++;
      if (it != _data.end()) cout << ",\t";
    }
    cout << ")" << endl;
  }
}
#endif
