// File: query.cpp
// Written by Joshua Green

#include "query.h"
#include "table.h"
#include "column.h"
#include "key.h"
#include "predicate.h"

#include <string>
#include <vector>
#include <iostream>
using namespace std;

class query;
struct column;
struct key_entry;

query::query(table *_table) {
  qtable = _table;
  key = qtable->get_key();
  _where = new predicate;
  _limit = -1;
}

query::query(const column &_key) {
  qtable = 0;
  key = _key;
  _where = new predicate;
  _limit = -1;
}

query::query() {
  qtable = 0;
  _where = new predicate;
  _limit = -1;
}

query::~query() {
  _where->clear();
  delete _where;
}

bool query::singularity(const column &col) const {
  return _where->col_required(col);
}

bool query::eval(const row &data) const {
  return _where->eval(data);
}

bool query::partial_eval(const column &col, const row &data) const {
  return _where->partial_eval(col, data);
}

query &query::where(const predicate &pred) {
  _where->clear();
  delete _where;
  _where = pred.copy();
  _where->_or = false;
  _limit = -1;
  return (*this);
}

query &query::where(bool _default) {
  _where->clear();
  return (*this);
}

query &query::limit(int count) {
  _limit = count;
  return (*this);
}

// Debug Functions:
void query::print() const {
  cout << "(";
  _where->print();
  cout << ")";
}
