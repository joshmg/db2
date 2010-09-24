// File: table.h
// Written by Joshua Green

#ifndef TABLE_H
#define TABLE_H

//-----------------------------------------------------//
// Define  DB_PROMPT  to prompt user at runtime before //
//   optimizing table file.                            //
//                                                     //
   #ifndef DB_PROMPT                                   //
     #define USR_PROMPT true                           //
     #define REQ_OPT(a) optimize()                     //
   #else                                               //
     #define USR_PROMPT _prompt()                      //
     #define REQ_OPT(a) _error_req_opt(#a)             //
   #endif                                              //
//                                                     //
//                                                     //
//-----------------------------------------------------//

//-----------------------------------------------------//
// Define the number of digits used to store integers. //
#define INDEX_MAX_SIZE 10                              //
//-----------------------------------------------------//


#include "fileio.h"
#include "row.h"
#include "column.h"
#include "query.h"
#include "key.h"
#include <string>
#include <map>
#include <vector>

// -------------------------------------------------------------- CLASS TABLE --------------------------------------------------------------- //
//   + initialize(string table_name)                                                                                                          //
//       - loads the table named by string and sets the tables internal variables                                                             //
//       - if the table doesn't exist, a table is created                                                                                     //
//   + close()                                                                                                                                //
//       - closes the table and writes all data to file                                                                                       //
//   + name()                                                                                                                                 //
//       - returns the name of the table                                                                                                      //
//   + size()                                                                                                                                 //
//       - returns the number of rows within the table                                                                                        //
//   + add_column(column)                                                                                                                     //
//       - inserts a column structure into the table                                                                                          //
//       - if the table is not empty before modifying its structure, optimization() must be called before accessing row data                  //
//       - if a default value is assigned within new_column, any old rows will inherit new_column's default value                             //
//       - if a default value is not assigned within new_column, any old rows will inherit an empty value                                     //
//   + edit_column(column old_column, column new_column)                                                                                      //
//       - replaces the column identically matched by old_column with the new_column                                                          //
//       - if the table is not empty before modifying its structure, optimization() must be called before accessing row data                  //
//       - any old rows inserted prior to a call to edit_column() will:                                                                       //
//           - be truncated to fit a smaller column size if set                                                                               //
//           - maintined data within the edited column if renamed                                                                             //
//   + delete_column(column deleted_column)                                                                                                   //
//       - deletes a column structure from the table                                                                                          //
//       - if the table is not empty before modifying its structure, optimization() must be called before accessing row data                  //
//       - any data from previous rows that was referenced by deleted_column will be removed                                                  //
//   + get_column(string col_name)                                                                                                            //
//       - retrieves a column which matches the referenced col_name                                                                           //
//       - if no column referenced by col_name is found within the table an empty column is returned                                          //
//   + get_structure()                                                                                                                        //
//       - returns a vector of type column which contains every column inside table                                                           //
//   + set_key()                                                                                                                              //
//       - sets the key column used within table for optimized searching                                                                      //
//       - changing the table's key requires a call to optimize() before accessing row data regardless the table being empty                  //
//   + add_row(row inserted_row)                                                                                                              //
//       - appends inserted_row into table                                                                                                    //
//       - any column fields that are not defined within inserted_row will assume the column's default value or remain empty                  //
//   + edit_row(row old_row, row new_row)                                                                                                     //
//       - replaces old_row with the data in new_row                                                                                          //
//       - any column fields that are not defined within new_row will assume old_row's data                                                   //
//       - returns the number of rows affected                                                                                                //
//       - NOTES:                                                                                                                             //
//           - column fields within new_row that are defined but empty will not assume old_row's data                                         //
//           - it is recommended, but not necessary, to use select() and the vector-overloaded edit_row()                                     //
//   + edit_row(vector<row> old_rows, row new_row)                                                                                            //
//       - performs edit_row(old_rows[i], row) for each row within old_rows                                                                   //
//       - NOTES:                                                                                                                             //
//           - it is recommended, but not necessary, to use select() and the vector-overloaded edit_row()                                     //
//   + read_row(int row_id)                                                                                                                   //
//       - returns the row associated with row_id                                                                                             //
//       - NOTES:                                                                                                                             //
//           - it is not recommended to use this function standalone as row_id is subject to change after a call to optimize()                //
//   + delete_row(int row_id)                                                                                                                 //
//       - removes the row associated with row_id from the table                                                                              //
//       - NOTES:                                                                                                                             //
//           - it is not recommended to use this function standalone as row_id is subject to change after a call to optimize()                //
//   + delete_row(row)                                                                                                                        //
//       - removes the row exactly matching all columns and data defined in row                                                               //
//       - NOTES:                                                                                                                             //
//           - this function calls the delete_row(int) if id is defined within row                                                            //
//           - if id is not defined in row,                                                                                                   //
//               delete_row(row) runs a query to return all rows matching the provided row, deleting the matching rows                        //
//   + delete_row(vector<row> rows)                                                                                                           //
//       - removes each individual row within rows from the table                                                                             //
//       - NOTES:                                                                                                                             //
//           - it is recommended, but not necessary, to use select() to acquire the rows vector                                               //
//   + select(query)                                                                                                                          //
//       - returns a vector of rows which is descibred by query                                                                               //
//       - NOTES:                                                                                                                             //
//           - it is recommended, but not necessary, to use select() for acquisition of all row vectors                                       //
//   + optimize()                                                                                                                             //
//       - rewrites the table data, structure, and key files to reflect any data and structure changes                                        //
//       - orders key entries to reflect optimized ordering                                                                                   //
//       - NOTES:                                                                                                                             //
//           - a call to optimize() is required after any structual changes to the table                                                      //
// ------------------------------------------------------------------------------------------------------------------------------------------ //

class table {
  private:
    bool _initialized;
    bool _require_optimize;
    fileio* _fstruct;
    fileio* _fdata;
    fileio* _fkeys;
    std::string _name;
    int _row_count;                                      // contains number of rows (including empty/deleted) in the file.
    int _deleted_row_count;                              // semi-cosmetic variable used only in size().
    int _row_size;
    column _key;                                         // _key is the table's key column (ie: column("id", 20)).
    std::vector<key_entry> _keys;                        // _keys contains the indexed rows (key_entry),
                                                         //      sorted (0 to 9 to a to z) by the row's column value
    
    bool _load_structure();
    void _load_keys();                                   // loads keys directly from key file. key file is only updated at optimize() and close().
    void _write_structure();
    void _calc_row_size();
    void _calc_rows();
    void _calc_deleted_rows();                           // heavy resourced function used only at initialize() to set initial _deleted_row_count.
    void _mark_size(std::string &data);
    void _write_keys();
    bool _write_row(std::map<std::string, std::string>&, // writes a row into _fdata at last _fdata.seek()'ed position.
                               bool enforce_flags=true); //      and only writes columns found within _Tstruct.
                                                         // ** modified the provided map to match the data written (ie. data changed via column flags)

    bool _prompt();
    void _error_req_opt(std::string);
    
    std::vector<column> _Tstruct;

  public:
    table();
    ~table();
    bool initialize(std::string filename);
    void close();
    std::string name();
    long int size();                    // returns the total number of rows in the table
    
    column add_column(const column &new_col);
    column edit_column(const column &old_col, column &new_col);
    void delete_column(const column &col);
    column get_column(const std::string &col_name) const;
    std::vector<column> get_structure() const;
    void set_key(const column &key);
    column get_key();

    row add_row(const row&);
    int edit_row(const row &old_row, row new_row);
    int edit_row(const std::vector<row> &, const row &new_row);
    row read_row(long int row_id);
    void delete_row(long int row_id);
    void delete_row(const row&);
    void delete_row(const std::vector<row>&);
    std::vector<row> select(const query&);

    void optimize();

    // Debug Functions:
    void print_structure();

};

#endif
