// File: row.h
// Written by Joshua Green

#ifndef ROW_H
#define ROW_H

#include "column.h"

#include <string>
#include <vector>
#include <map>

#ifndef DB_LITE
class table;
#endif

// --------------------------------------------------------------- STRUCT ROW --------------------------------------------------------------- //
//   + add(column col, string data)                                                                                                           //
//       - defines row's entry referenced by col.name as data                                                                                 //
//   + add(string col_name, string data)                                                                                                      //
//       - defines row's entry referenced by col_name as data                                                                                 //
//       - this version of add(string, string) is functionally equivalent to add(col, string)                                                 //
//   + operator[column]                                                                                                                       //
//       - returns the data defined in row that is referenced by col.name                                                                     //
//   + operator[string col_name]                                                                                                              //
//       - returns the data defined in row that is referenced by col_name                                                                     //
//   + clear()                                                                                                                                //
//       - resets the row as if it were just constructed                                                                                      //
//   + is_empty()                                                                                                                             //
//       - returns true if there are no defined entries within row                                                                            //
//   + get_id()                                                                                                                               //
//       - returns the row id which can only be set by a table's select() function                                                            //
//   + is_defined(column)                                                                                                                     //
//       - returns true if there is a definition for the specified column                                                                     //
//       - is_defined() does distinguish between an empty definition and an entry that is not defined                                         //
//   + is_defined(string column_name)                                                                                                         //
//       - returns true if there is a definition for the specified column_name                                                                //
//       - is_defined() does distinguish between an empty definition and an entry that is not defined                                         //
//   + print(table*)                                                                                                                          //
//       - outputs a human-understandable representation of all definitions within the row to standard out                                    //
//       - table* is optional but provides a much improved representation of the row including formatted entry sizes                          //
// ------------------------------------------------------------------------------------------------------------------------------------------ //
struct row {
  private:
  	int id;
    std::map<std::string, std::string> _data;
    row(int);
  	friend class table;
  	
  public:
    row();
    ~row();
    row &add(const std::string &key, const std::string &data);
    row &add(const column &col, const std::string &data);
    row &remove(const std::string& key);
    row &remove(const column& col);
    std::string operator[](const column&) const;
    std::string operator[](const std::string&) const;
    void clear();
    bool is_empty() const;
    int get_id() const;
    bool is_defined(const column&) const;
    bool is_defined(const std::string&) const;

    std::string to_str() const;
    row& from_str(const std::string&);

    // Debug Functions:
    #ifndef DB_LITE
    void print(table* t=0) const;
    #endif
};

#endif
