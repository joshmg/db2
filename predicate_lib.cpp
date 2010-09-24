// File: predicate_lib.cpp
// Written by Joshua Green

#include "predicate.h"
#include "predicate_lib.h"
#include "column.h"
#include "str.h"
#include <string>
#include <cstdlib>

using namespace std;

greaterthan::greaterthan(const column &col, const string &expected_value, bool _not) {
  _col = col;
  _expected_value = expected_value;
  _invert = _not;
  _or = false;
  _print_symbol = ">";
}

greaterthan* greaterthan::copy() const {
  greaterthan *p = new greaterthan(_col, _expected_value, _invert);
  p->_or = _or;
  return p;
}

bool greaterthan::eval(const row &data) const {
  if (is_numeric(_expected_value)) {
    if (!is_numeric(data[_col.name])) return (_invert^false);
    return (_invert^(atof(data[_col.name].c_str()) > atof(_expected_value.c_str())));
  }
  return (_invert^(!(strlessthan(data[_col.name], _expected_value)) && (data[_col.name] != _expected_value) ));
}

similarto::similarto(const column &col, const string &expected_value, bool _not) {
  _col = col;
  _expected_value = expected_value;
  _invert = _not;
  _or = false;
  _print_symbol = "~=";
  
  THRESHOLD = 0.25;
}

similarto* similarto::copy() const {
  similarto *p = new similarto(_col, _expected_value, _invert);
  p->_or = _or;
  return p;
}

float similarto::_score(const string &str) const {
  if (str.length() == 0) return 0; // avoid div by zero
  
  float value = 0;
  for (int i=0;i<str.length();i++) {
    char c = str[i];
    // alpha
    if ((c > 64 && c < 91) || (c > 96 && c < 123)) {
      c = tolower(c);
      value = c-(97+10); // a = 10, b = 11, etc...
    }
    else {
      // numeric
      if (c > 47 && c < 58) value += c-48; // 0 = 0, 1 = 1, etc...
    }
  }
  return (value/str.length());
}

bool similarto::eval(const row &data) const {
  float expected_score = _score(_expected_value);
  return ( _invert^(fabs(_score(data[_col.name]) - expected_score) < expected_score*THRESHOLD) );
}
