// File: db.cpp
// Written by Joshua Green

#include "db.h"
#include "fileio.h"
#include "table.h"
#include "row.h"

#include <string>
#include <vector>
#include <iostream>
using namespace std;

database::database() {
  _initialized = false;
}

database::~database() {
  if (_initialized) close();
}

bool database::initialize(string db_name) {
  _name = db_name;
  string file_name = db_name + ".db";
  _db.open(file_name);
  string buff;

  _db.seek(0);
  long long size = _db.size();
  if (size > 0) {
    while (_db.pos() < size) {
      buff = _db.read(-1, (char)0x11);
      if (buff.length() > 0) {
        table *t = new table;
        t->initialize(buff);
        _tables.push_back(t);
      }
    }
    buff.clear();
  }
  _initialized = true;
  return true;  
}

void database::close() {
  if (_initialized) {
    _write_db();
    _db.close();
    while (_tables.size() > 0) {
      (*_tables.begin())->close();
      delete (*_tables.begin());
      _tables.erase(_tables.begin());
    }
    _initialized = false;
  }
}

table* database::add_table(const string &name) {
  table *temp_table = new table;
  temp_table->initialize(name);
  _tables.push_back(temp_table);
  return temp_table;
}

table* database::get_table(const string &name) {
  for (int i=0;i<_tables.size();i++) if (name == _tables[i]->name()) return _tables[i];
  return 0;
}

string database::name() {
  return _name;
}

void database::_write_db() {
  string filename = name();
  filename += ".db";
  
  _db.rm();
  _db.open(filename, "w+");
  for (int i=0;i<_tables.size();i++) {
    _db.write(_tables[i]->name());
    _db.write((char)0x11);
  }
}

vector<row> database::refine(const query &q, const vector<row> &rows) {
  vector<row> results;

  for (int i=0;i<rows.size();i++) {
    if (q.eval(rows[i]) && !rows[i].is_empty()) results.push_back(rows[i]);
  }

  return results;
}
