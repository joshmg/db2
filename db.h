// File: db.h
// Written by Joshua Green

#ifndef DB_H
#define DB_H

#include "fileio.h"
#include "table.h"
#include "row.h"
#include <string>
#include <vector>

#pragma comment(lib, "db.lib")

// ------------------------------------------------------------- CLASS DATABASE ------------------------------------------------------------- //
//   + initialize(string database_name)                                                                                                       //
//       - collects the database by name if's previously created, if it's not created then one is made                                        //
//   + name()                                                                                                                                 //
//       - returns the name of the database                                                                                                   //
//   + add_table(string table_name)                                                                                                           //
//       - creates a table and inserts it into the database                                                                                   //
//       - the table is named and referenced by the string provided                                                                           //
//       - the created table is initialzed and ready for use after a call to add_table                                                        //
//       - a pointer to table is returned by add_table()                                                                                      //
//   + get_table(string table_name)                                                                                                           //
//       - retrieves a table which matches the referenced name                                                                                //
//       - returns a null pointer if the table does not exist                                                                                 //
//   + refine(query, vector<row>)                                                                                                             //
//       - applies a query to a set of previously selected set of rows and returns the new set                                                //
//   + NOTES:                                                                                                                                 //
//       - tracks tables within the database                                                                                                  //
//       - most database functionality is independantly contained within table                                                                //
// ------------------------------------------------------------------------------------------------------------------------------------------ //
class database {
  private:
    fileio _db;
    std::string _name;
    std::vector<table*> _tables;
    void _write_db();                           // writes the db file
    bool _initialized;

  public:
    database();
    ~database();
    bool initialize(std::string db_name);
    void close();
    std::string name();

    table* add_table(const std::string &name);
    table* get_table(const std::string &name);

    std::vector<row> refine(const query&, const std::vector<row> &rows);
};

#endif
