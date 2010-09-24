// File: column.h
// Written by Joshua Green

#ifndef COLUMN_H
#define COLUMN_H

#include <string>

// ------------------------------------------------------------- STRUCT COLUMN -------------------------------------------------------------- //
//   + ie: column("name", 30)                                                                                                                 //
//   + data structure containing:                                                                                                             //
//       - column's name                                                                                                                      //
//       - column's size                                                                                                                      //
//   + set_default(string)                                                                                                                    //
//       - if row[column] is left unassigned at time of insertion, row[column] is assigned the specified default value                        //
//   + auto_inc()                                                                                                                             //
//       - column's default value is incrimented 1 upon row insertion starting at the default value                                           //
//       - if row[column] value is left unassigned, the incrimented value is assigned to the respective column value within inserted row      //
//   + not_null(bool)                                                                                                                         //
//       - true enables not_null, while false disables not_null                                                                               //
//       - prevents a row from being inserted if the row[column] value is left unassigned                                                     //
//       - will function with auto_inc                                                                                                        //
//       - a row prevented from insertion via not_null still incriments all collumns with the auto_inc attribute                              //
//   + clear()                                                                                                                                //
//   + compare operator return true if both column names and sizes are equivalent                                                             //
//   + NOTES:                                                                                                                                 //
//       - unless the column is initially being created to set a table's structure,                                                           //
//           it is recommended to retrieve a table's column via table's get_column(string) or get_structure()                                 //
//       - third constructor parameter is used internally by table class to set column flags when loading from file                           //
//       - flags are defined below class declaration. class typing, although noted, is not implimented.                                       //
// ------------------------------------------------------------------------------------------------------------------------------------------ //
struct column {
  std::string name, alias;
  long int size;
  bool flags[26];
  std::string _default;

  column();
  column(const std::string &_name, long int _size, std::string set_flags="");
  ~column();
  void clear();

  void _load_flags(const std::string&);     // set flags according to turn provided ready-to-print string
  std::string _encode_flags();              // return ready-to-print string version of flags

  column &set_default(const std::string &value="");

  bool operator==(const column &comp) const;
  bool operator==(const column *comp) const;
  bool operator!=(const column &comp) const;
  bool operator!=(const column *comp) const;
  
  // attribute flag options:
  column &auto_inc(bool value=true);
  bool is_auto_inc();
  column &not_null(bool value=true);
  bool is_not_null();
  
};

  /* Column Flags:
  00 097 a: auto incriment
  01 098 b: 
  02 099 c: char
  03 100 d: double
  04 101 e: 
  05 102 f: float
  06 103 g: 
  07 104 h: 
  08 105 i: int
  09 106 j: 
  10 107 k: 
  11 108 l: long int
  12 109 m: 
  13 110 n: not null
  ...
  */
  
#endif
