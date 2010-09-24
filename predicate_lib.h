// File: predicate_lib.h
// Written by Joshua Green

#ifndef PREDICATE_LIB_H
#define PREDICATE_LIB_H

#include "predicate.h"
#include "column.h"
#include <string>

// ----------------------------------------------------------- PREDICATE LIBRARY  ----------------------------------------------------------- //
//   + to create a user-defined predicate expansion:                                                                                          //
//     - class must inherit public condition                                                                                                  //
//     - must have a copy function which returns a const pointer of its own type (created with the c++ new function)                          //
//     - must provide a defintion for eval which takes a type of const row reference                                                          //
//     - must provide a constructor taking three arguments: const column reference, const string reference, defaulted bool                    //
//     - may provide a reimplimentation of the Not() function which inverts the value when eval() is utilized                                 //
//   + predicates have access to protected condition members:                                                                                 //
//     - column _col                                                                                                                          //
//       - contains the concerned column                                                                                                      //
//     - string _expected_value                                                                                                               //
//       - contains the value which row[_col] should access during eval()                                                                     //
//     - string _print_symbol                                                                                                                 //
//       - the symbol used to represent the predicate when printed to the screen                                                              //
//     - bool _invert                                                                                                                         //
//       - used to facilitate the inversion of eval() when the Not() function is utilized                                                     //
// ------------------------------------------------------------------------------------------------------------------------------------------ //
class greaterthan : public condition {
  protected:
    greaterthan* copy() const;
  public:
    greaterthan(const column &col, const std::string& expected_value, bool _not=false);
    bool eval(const row&) const;
};

class similarto : public condition {
  protected:
    similarto* copy() const;
  private:
    float _score(const std::string&) const;
    float THRESHOLD;
  public:
    similarto(const column &col, const std::string& expected_value, bool _not=false);
    bool eval(const row&) const;
};

#endif
