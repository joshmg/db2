// File: key.h
// Written by Joshua Green

#ifndef KEY_H
#define KEY_H

#include "column.h"
#include "str.h"

#include <string>

struct column;
struct condition;

// --------------------------------------------------------------- KEY ENTRY ---------------------------------------------------------------- //
//   + NOTES:                                                                                                                                 //
//      - used to contain the individual row's key column entries with their respective file positions inside *.dat	                          //
//      - contains:                                                                                                                           //
//        - a single row's value for the table's key column                                                                                   //
//        - the row's associated file position within the table's *.dat file                                                                  //
//        - a pointer to the table's key column                                                                                               //
// ------------------------------------------------------------------------------------------------------------------------------------------ //
struct key_entry {
  column *col;            // pointer to the table's key column
  std::string data;       // the row's value for the table's key column
  long long int fpos;     // file position inside *.dat where row starts
  long int row_id;
  
  key_entry();
  key_entry(column *_key, std::string _data, long long int _fpos, long int _id=0);
  ~key_entry();
  
  bool operator==(const key_entry &comp);
};

#endif
