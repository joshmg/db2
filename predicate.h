// File: predicate.h
// Written by Joshua Green

#ifndef PREDICATE_H
#define PREDICATE_H

#include "column.h"
#include "row.h"
#include <string>
#include <vector>

class predicate;
class condition;

// ------------------------------------------------------------- CLASS PREDICATE ------------------------------------------------------------ //
//   + copy()                                                                                                                                 //
//       - copy is the only way a predicate should be copied due to internal pointers                                                         //
//       - returns a pointer to a dynamically created instance of copy that must be deleted to avoid memory leaks                             //
//   + eval(row)                                                                                                                              //
//       - logically evaluates the predicate using given row                                                                                  //
//   + size()                                                                                                                                 //
//       - returns the number of total number of conditions in the predicate, including the predicates nested conditions                      //
//   + And(predicate) / And(condition)                                                                                                        //
//       - logically ands any previous predicates/conditions within the predicate with an additional predicate/condition                      //
//       - uppercased because "and" is a keyword on some compilers                                                                            //
//   + add(predicate) / add(condition)                                                                                                        //
//       - an alias of And                                                                                                                    //
//   + Or(predicate) / Or(condition)                                                                                                          //
//       - logically ors any previous predicates/conditions within the predicate with an additional predicate/condition                       //
//       - uppercased because "or" is a keyword on some compilers                                                                             //
//   +  Not(predicate) / Not(condition)                                                                                                       //
//       - logically inverts any the associated predicate                                                                                     //
//       - uppercased because "not" is a keyword on some compilers                                                                            //
//   +  print()                                                                                                                               //
//       - outputs a string representation of the predicate                                                                                   //
// ------------------------------------------------------------------------------------------------------------------------------------------ //
class predicate {
  private:
    std::vector<predicate*> _pred_list;
    std::vector<condition*> _conditions;
    bool _invert;
    
    bool col_required(const column&) const; // used by table class to determine if key can be used for a key-search
    bool partial_eval(const column&, const row&) const;
    void extract(const column&);
    friend class table;
    friend class query;

  public:
    bool _or;
    
    predicate(bool _not=false);
    ~predicate();
    predicate* copy() const;
    predicate &clear();
    bool eval(const row&) const;
    int size() const;

    predicate &add(const predicate&);
    predicate &add(const condition&);

    predicate &And(const predicate&);
    predicate &And(const condition&);

    predicate &Or(const predicate&);
    predicate &Or(const condition&);
    
    predicate &Not(bool value=true);

    void print() const;
};

class condition {
  protected:
    column _col;
    std::string _expected_value;
    std::string _print_symbol;
    bool _invert;
    
    friend class predicate;
    friend class query;
    
  public:
    condition();
    virtual condition* copy() const = 0;
    bool _or;
    virtual condition &Not(bool value=true);
    virtual bool eval(const row&) const; // avoiding abstract class: function returns false
};

class equalto : public condition {
  protected:
    equalto* copy() const;
  public:
    equalto(const column &col, const std::string& expected_value, bool _not=false);
    bool eval(const row&) const;
};

class lessthan : public condition {
  protected:
    lessthan* copy() const;
  public:
    lessthan(const column &col, const std::string& expected_value, bool _not=false);
    bool eval(const row&) const;
};

#endif
