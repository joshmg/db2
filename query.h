// File: query.h
// Written by Joshua Green

#ifndef QUERY_H
#define QUERY_H

#include "column.h"
#include "key.h"
#include "predicate.h"
#include <vector>

struct column;
struct key_entry;

// --------------------------------------------------------------- CLASS QUERY -------------------------------------------------------------- //
//   + where()                                                                                                                                //
//       - sets the query's predicate                                                                                                         //
//       - refer to predicate class in order to create a tailored predicate                                                                   //
//   + eval(row)                                                                                                                              //
//       - returns true if the row satisfies the defined predicate                                                                            //
//   + limit(int)                                                                                                                             //
//       - limits the quantity of rows that a query will return when passed to select()                                                       //
//   + print()                                                                                                                                //
//       - outputs a human-understandable representation of the query's predicate definition to standard out                                  //
// ------------------------------------------------------------------------------------------------------------------------------------------ //
class query {
  private:
    column key;                   // the key column used for efficient table-key searches
    predicate *_where;
    int _limit;

    friend class table;           // allow table access to private variables
    table* qtable;                // pointer to table associated with the query

    // singularity returns true if the column parameter is a necessary precondition for the query
    bool singularity(const column&) const;      //   example: WHERE (id = '1') AND (name = '1')
                                                //     singularity(id)   = true
                                                //     singularity(name) = true
                                                //   example: WHERE (id = '1') OR (name = '1')
                                                //     singularity(id)   = false
                                                //     singularity(name) = false

    // evaluates only the assigned predicates of a specific column on the row to return an ultimate value
    bool partial_eval(const column&, const row&) const;

  public:
    query(table *_table);
    query(const column &_key);
    query();
    ~query();

    query &where(const predicate&);
    query &where(bool _default=true);

    // evaluates all of the assigned predicates on a row to return an ultimate value
    bool eval(const row&) const;

    query &limit(int count);

    void print() const;
};

#endif
