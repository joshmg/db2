// File: predicate.cpp
// Written by Joshua Green

#include "predicate.h"
#include "row.h"
#include "str.h"
#include <string>
#include <cmath>
#include <iostream>
#include <cstdlib>
using namespace std;

bool predicate::col_required(const column &col) const {
  bool col_is_and = false, all_the_same = true, ors_exist = false;
  if (_conditions.size()) all_the_same = false;
  bool original_col_set = false;
  column original_col;
  
  for (int i=0;i<_conditions.size();i++) {
    if (!original_col_set) {
      original_col_set = true;
      original_col = _conditions[i]->_col; // store first found column
    }
    if (original_col != _conditions[i]->_col) all_the_same = false;
    
    if (col == _conditions[i]->_col) {
      if (!_conditions[i]->_or) col_is_and = true;
    }
    else {
      if (_conditions[i]->_or) ors_exist = true;
    }
  }
  bool nested_required = false;
  for (int i=0;i<_pred_list.size();i++) {
    if (_pred_list[i]->_or) ors_exist = true;
    nested_required = _pred_list[i]->col_required(col);
  }
  
  return ( (((col_is_and || nested_required) && (!ors_exist)) || (all_the_same)) );
}

predicate::predicate(bool _not) {
  _invert = _not;
  _or = false;
}

predicate::~predicate() {
  for (int i=0;i<_pred_list.size();i++) {
    _pred_list[i]->clear();
    delete _pred_list[i];
  }
  for (int i=0;i<_conditions.size();i++) delete _conditions[i];
}

predicate *predicate::copy() const {
  predicate *p = new predicate;
  for (int i=0;i<_pred_list.size();i++) p->_pred_list.push_back(_pred_list[i]->copy());
  for (int i=0;i<_conditions.size();i++) p->_conditions.push_back(_conditions[i]->copy());
  p->_invert = _invert;
  p->_or = _or;
  return p;
}

/*predicate *predicate::copy(const predicate& tocopy) const {
  predicate *p = new predicate;
  for (int i=0;i<tocopy._pred_list.size();i++) p->_pred_list.push_back(copy(tocopy._pred_list[i]));
  for (int i=0;i<tocopy._conditions.size();i++) p->_conditions.push_back(tocopy._conditions[i]->create());
  p->_invert = tocopy._invert;
  p->_or = tocopy._or;
  return p;
}*/

predicate &predicate::clear() {
  for (int i=0;i<_pred_list.size();i++) {
    _pred_list[i]->clear();
    delete _pred_list[i];
  }
  _pred_list.clear();
  
  for (int i=0;i<_conditions.size();i++) delete _conditions[i];
  _conditions.clear();

  _invert = false;
  _or = false;
  return (*this);
}

bool predicate::eval(const row &data) const {
  bool and_set = false, or_set = false;
  bool and_results = true, or_results = false;
  for (int i=0;i<_conditions.size();i++) {
    bool evaluated_value = _conditions[i]->eval(data);
    if (_conditions[i]->_or) {
      or_results = (or_results || evaluated_value);
      or_set = true;
    }
    else {
      and_results = (and_results && evaluated_value);
      and_set = true;
    }
  }
  for (int i=0;i<_pred_list.size();i++) {
    bool evaluated_value = _pred_list[i]->eval(data);
    if (_pred_list[i]->_or) {
      or_results = (or_results || evaluated_value);
      or_set = true;
    }
    else {
      and_results = (and_results && evaluated_value);
      and_set = true;
    }
  }
  //                                                                   (no conditions defaults to true)
  return ( (_invert^((and_results && and_set) || (or_results && or_set))) || (!and_set && !or_set) );
}

bool predicate::partial_eval(const column &col, const row& data) const {
  predicate *p = copy();
  p->extract(col);
  bool value = p->eval(data);
  p->clear(); // there are memory leaks without clearing before deleting!
  delete p;
  return value;
}

void predicate::extract(const column &col) {
  for (int i=0;i<_conditions.size();i++) {
    if (_conditions[i]->_col != col) {
      delete _conditions[i];
      _conditions.erase(_conditions.begin()+i);
      --i;
    }
  }

  for (int i=0;i<_pred_list.size();i++) {
    _pred_list[i]->extract(col);
    if (_pred_list[i]->size() == 0) {
      _pred_list[i]->clear();
      delete _pred_list[i];
      _pred_list.erase(_pred_list.begin()+i);
      --i;
    }
  }
}

int predicate::size() const {
  int return_value = _conditions.size();
  for (int i=0;i<_pred_list.size();i++) {
    return_value += _pred_list[i]->size();
  }
  return return_value;
}

predicate &predicate::add(const predicate &pred) {
  return this->And(pred);
}
predicate &predicate::add(const condition &cond) {
  return this->And(cond);
}

predicate &predicate::And(const predicate &pred) {
  _pred_list.push_back(pred.copy());
  _pred_list.back()->_or = false;
  return (*this);
}
predicate &predicate::And(const condition &cond) {
  condition *c = cond.copy();
  _conditions.push_back(c);
  _conditions.back()->_or = false;
  return (*this);
}

predicate &predicate::Or(const predicate &pred) {
  _pred_list.push_back(pred.copy());
  _pred_list.back()->_or = true;
  return (*this);
}
predicate &predicate::Or(const condition &cond) {
  condition *c = cond.copy();
  _conditions.push_back(c);
  _conditions.back()->_or = true;
  return (*this);
}

predicate &predicate::Not(bool value) {
  _invert = value;
  return (*this);
}

condition::condition() {
  _invert = false;
}

condition &condition::Not(bool value) {
  _invert = value;
  return (*this);
}

bool condition::eval(const row& data) const {
  cout << "WARNING: default virtual eval called." << endl;
  return false;
}

equalto::equalto(const column &col, const string &expected_value, bool _not) {
  _col = col;
  _expected_value = expected_value;
  _invert = _not;
  _or = false;
  _print_symbol = "==";
}

equalto* equalto::copy() const {
  equalto *p = new equalto(_col, _expected_value, _invert);
  p->_or = _or;
  return p;
}

bool equalto::eval(const row &data) const {
  return (_invert^(data[_col.name] == _expected_value));
}

lessthan::lessthan(const column &col, const string &expected_value, bool _not) {
  _col = col;
  _expected_value = expected_value;
  _invert = _not;
  _or = false;
  _print_symbol = "<";
}

lessthan* lessthan::copy() const {
  lessthan *p = new lessthan(_col, _expected_value, _invert);
  p->_or = _or;
  return p;
}

bool lessthan::eval(const row &data) const {
  if (is_numeric(_expected_value)) {
    if (!is_numeric(data[_col.name])) return (_invert^false);
    return (_invert^(atof(data[_col.name].c_str()) < atof(_expected_value.c_str())));
  }
  return (_invert^(strlessthan(data[_col.name], _expected_value)));
}

// Debug Function:
void predicate::print() const {
  bool first = true;
  if (_invert) cout << "!(";
  for (int i=0;i<_conditions.size();i++) {
    if (!first) {
      if (_conditions[i]->_or) cout << " or ";
      else cout << " and ";
    }
    else first = false;

    if (_conditions[i]->_invert) cout << "!";
    cout << "(" << _conditions[i]->_col.name << " " << _conditions[i]->_print_symbol << " " << _conditions[i]->_expected_value << ")";
  }
  
  for (int i=0;i<_pred_list.size();i++) {
    if (!first) {
      if (_pred_list[i]->_or) cout << " or ";
      else cout << " and ";
    }
    else first = false;
    
    cout << "(";
    _pred_list[i]->print();
    cout << ")";
  }
  if (_invert) cout << ")";
}
