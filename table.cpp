// File: table.cpp
// Written by Joshua Green

#include "fileio.h"
#include "table.h"
#include "str.h"
#include "key.h"
#include "query.h"
#include <string>
#include <map>
#include <algorithm>
#include <iostream>

using namespace std;

bool key_alpha_sort(key_entry a, key_entry b);

table::table() {
  _initialized = false;
  _fdata = new fileio;
  _fkeys = new fileio;
  _fstruct = new fileio;
}

table::~table() {
  if (_initialized) close();
  delete _fdata;
  delete _fkeys;
  delete _fstruct;
}

bool table::initialize(string filename) {
  _name = filename;
  filename += ".str";
  _fstruct->open(filename);
  if (!_fstruct->is_open()) _fstruct->open(filename, "wb+");
  
  filename = _name;
  filename += ".dat";
  _fdata->open(filename);
  if (!_fdata->is_open()) _fdata->open(filename, "wb+");
  
  filename = _name;
  filename += ".key";
  _fkeys->open(filename);
  if (!_fkeys->is_open()) _fkeys->open(filename, "wb+");
  
  _initialized = _fstruct->is_open() && _fdata->is_open() && _fkeys->is_open();
  if (_initialized) {
    _require_optimize = false;

    _load_structure();
    _calc_rows();
    _calc_deleted_rows();
    _load_keys();
  }
  
  return _initialized;
}

void table::close() {
  if (_initialized) {
    if (_require_optimize) optimize();
    else {
      _write_keys();
      _write_structure();
    }
    _initialized = false;
    _fdata->close();
    _fkeys->close();
    _fstruct->close();
  }
    
  // clear all internal variables:
  _name.clear();
  _key.clear();
  _keys.clear();
  _row_count = 0;
  _deleted_row_count = 0;
  _row_size = 0;
  _Tstruct.clear();
}

string table::name() {
  return _name;
}

long int table::size() {
  if (!_initialized) return 0;
  return _row_count-_deleted_row_count;
}

bool table::_load_structure() {
  _Tstruct.clear();
  _fstruct->seek(0);
  while (_fstruct->pos() < _fstruct->size()) {
    column temp_col;
    // load column name
    temp_col.name = _fstruct->read(-1, (char)0x12);
    temp_col.alias = temp_col.name;
    // load column size
    temp_col.size = atol(_fstruct->read(-1, (char)0x12).c_str());
    // load column flags
    temp_col._load_flags(_fstruct->read(-1, (char)0x12));
    // load column default value
    temp_col._default = _fstruct->read(-1, (char)0x12);
    _Tstruct.push_back(temp_col);
    _fstruct->seek(_fstruct->pos()+1); // skip end of column marker ((char)0x11)
  }
  _calc_row_size();
  return true;
}

column table::add_column(const column &new_col) {
  if (_row_count > 0) if (!USR_PROMPT) return column();
  _Tstruct.push_back(new_col);
  _write_structure();
  _require_optimize = true;
  
  if (!_require_optimize) {
    _calc_row_size();
    _calc_rows();
  }
  return new_col;
}

column table::edit_column(const column &old_col, column &new_col) {
  if (_row_count > 0) if (!USR_PROMPT) return old_col;
  for (int i=0;i<_Tstruct.size();i++) {
    if (_Tstruct[i] == old_col) {
      new_col.alias = old_col.name; // facilitates renaming a structure
      _Tstruct[i] = new_col;
      _write_structure();
      if (_row_count > 0) _require_optimize = true;
      
      if (!_require_optimize) {
        _calc_row_size();
        _calc_rows();
      }
      return new_col;
    }
  }
  return old_col;
}

void table::delete_column(const column &col) {
  if (_row_count > 0) if (!USR_PROMPT) return;
  for (int i=0;i<_Tstruct.size();i++) {
    if (_Tstruct[i] == col) {
      _Tstruct.erase(_Tstruct.begin()+i);
      _write_structure();
      if (_row_count > 0) _require_optimize = true;
      break;
    }
  }
  if (!_require_optimize) {
    _calc_row_size();
    _calc_rows();
  }
}

column table::get_column(const std::string &col_name) const {
  for (int i=0;i<_Tstruct.size();i++) {
    if (_Tstruct[i].name == col_name) return _Tstruct[i];
  }
  return column();
}

vector<column> table::get_structure() const {
  return _Tstruct;
}

void table::set_key(const column &key_column) {
  if (_row_count > 0) if (!USR_PROMPT) return;
  _key = key_column;
  _require_optimize = true;
}

column table::get_key() {
  return _key;
}

void table::_write_structure() {
  string fname = _fstruct->filename();
  _fstruct->rm();
  _fstruct->open(fname, "wb+");
  for (int i=0;i<_Tstruct.size();i++) {
    // column name
    _fstruct->write(_Tstruct[i].name);
    _fstruct->write(0x12);
    // column size
    _fstruct->write(itos(_Tstruct[i].size));
    _fstruct->write(0x12);
    // column property flags
    _fstruct->write(_Tstruct[i]._encode_flags());
    _fstruct->write(0x12);
    // default value:
    _fstruct->write(_Tstruct[i]._default);
    _fstruct->write(0x12);
    _fstruct->write(0x11);
  }
}

row table::add_row(const row& row_data) {
  row data = row_data;
  if (_require_optimize) {
    //_error_req_opt("create new row");
    REQ_OPT("create new row");
    if (_require_optimize) return data;
  }
  
  _fdata->seek("END");
  // write row, if row aborts, don't add key entry
  if (_write_row(data._data)) _keys.push_back( key_entry(&_key, data[_key.name], _fdata->pos()-_row_size, _row_count) );
  _calc_rows();
  return data;
}

int table::edit_row(const row &old_row, row new_row) {
  int count = 0;
  // add column data that is not included to new_row (still enables writing empty values)
  for (int i=0;i<_Tstruct.size();i++) {
    if (!new_row.is_defined(new_row[_Tstruct[i].name])) new_row.add(_Tstruct[i], old_row[_Tstruct[i].name]);
  }
  
  if (old_row.id >= 0) {
    _fdata->seek(old_row.id*_row_size);
    _write_row(new_row._data);
    count++;
    
    // if old row's key value has changed then update the key entry (insde _keys)
    if (old_row[_key.name] != new_row[_key.name]) {
      for (int i=0;i<_keys.size();i++) {
        if (_keys[i].row_id == old_row.id) {
          _keys[i].data = new_row[_key.name]; // change key entry data to represent change
          break;
        }
      }
    }
    
  }
  else {
    query q;
    predicate p;

    for (int i=0;i<_Tstruct.size();i++) p.And( equalto(_Tstruct[i], old_row[_Tstruct[i].name]) ); // build predicate matching the row

    q.where(p);
    vector<row> results = select(q);
    for (int i=0;i<results.size();i++) {
      _fdata->seek(results[i].id * _row_size);
      _write_row(new_row._data);
      count++;
      
      // if old row's key value has changed then update the key entry (insde _keys)
      if (old_row[_key.name] != new_row[_key.name]) {
        for (int i=0;i<_keys.size();i++) {
          if (_keys[i].row_id == results[i].id) {
            _keys[i].data = new_row[_key.name]; // change key entry data to represent change
            break;
          }
        }
      }
      
    }
  }
  return count;
}

int table::edit_row(const vector<row> &old_rows, const row &new_row) {
  int count = 0;
  for (int i=0;i<old_rows.size();i++) {
    count += edit_row(old_rows[i], new_row);
  }
  return count;
}

bool table::_write_row(map<string, string> &data, bool enforce_flags) {
  bool abort = false;
  map<string, string>::iterator it;
  string write_buffer;
  for (int i=0;i<_Tstruct.size();i++) {
    // incriment column default if auto-inc
    if (_Tstruct[i].is_auto_inc() && enforce_flags) _Tstruct[i]._default = itos(atoi(_Tstruct[i]._default.c_str())+1);
    
    // only keep data found in table's columns,
    //   and add column data not found the given row
    it = data.find(_Tstruct[i].name);
    if (it == data.end()) {
      data.insert(pair<string,string>(_Tstruct[i].name, _Tstruct[i]._default));
      it = data.find(_Tstruct[i].name);
    }
    
    // if set, enforce not_null flag
    if (it->second.length() == 0 && _Tstruct[i].is_not_null() && enforce_flags) abort = true;
    
    // truncate data if over size allocation: (truncates end)
    if (it->second.length() > _Tstruct[i].size) it->second.erase(_Tstruct[i].size, it->second.length() - _Tstruct[i].size);
    
    // append column name and data fields to the write buffer
    write_buffer += _Tstruct[i].name;
    write_buffer += 0x12;
    write_buffer += it->second;
    write_buffer.append(_Tstruct[i].size-it->second.length(), '\0');
    write_buffer += 0x12;
  }
  write_buffer += 0x11; // add end of row marker
  _mark_size(write_buffer); // mark the row size
  
  // write the write_buffer if not aborted
  if (!abort) _fdata->write(write_buffer);
  
  return (!abort);
}

void table::_mark_size(string &data) {
  string size;
  size += itos(data.length()+INDEX_MAX_SIZE);
  size.insert(0, INDEX_MAX_SIZE-size.length(), '0');
  data.insert(0, size);
}

void table::_write_keys() {
  string filename = _fkeys->filename();
  _fkeys->close();
  filename = _name;
  filename += ".key";
  _fkeys->open(filename, "wb+");
  
  // write key column as header in *.key
  _fkeys->write(_key.name);
  _fkeys->write(0x12);
  _fkeys->write(itos(_key.size));
  _fkeys->write(0x12);
  _fkeys->write(0x11);

  // sort the keys in case-exempt alphabetical order
  sort(_keys.begin(), _keys.end(), key_alpha_sort);

  // write entries
  for (long int i=0;i<_keys.size();i++) {
    _fkeys->write(_keys[i].data);
    _fkeys->write(0x12);
    _fkeys->write(itos(_keys[i].fpos));
    _fkeys->write(0x12);
    _fkeys->write(0x11);
  }
}

void table::_calc_row_size() {  
  _row_size = INDEX_MAX_SIZE;         // compensate for row size "00000000"
  for (int i=0;i<_Tstruct.size();i++) {
    _row_size += _Tstruct[i].name.length();  // compensate for column name
    _row_size += _Tstruct[i].size;           // compensate for column data
    _row_size += 2;                                       // compensate for end of field markers
  }
  _row_size++;                                            // compensate for end of row marker
}

void table::_calc_rows() {
  _calc_row_size();
  _row_count = _fdata->size()/_row_size;
}

void table::_calc_deleted_rows() {
  _deleted_row_count = 0;
  if (_row_count < 1) _calc_rows();
  for (int i=0;i<_row_count;i++) {
    if(read_row(i).is_empty()) _deleted_row_count++;
  }
}

row table::read_row(long int row_id) {
  if (!_initialized) return row();
  if (_require_optimize) {
    REQ_OPT("read row");
    if (_require_optimize) return row();
  }
  if (row_id > _row_count) return row();
  
  row return_row(row_id);
  _fdata->seek(_row_size*row_id);
  long int read_size = atol(_fdata->read(INDEX_MAX_SIZE).c_str())-(INDEX_MAX_SIZE+1); // INDEX_MAX_SIZE for the size of the row, 1 for row end marker

  string buffer, col_name, col_data;
  long int temp;
  while (read_size > 0) {

    // get and parse column name
    buffer = _fdata->read(_row_size, (char)0x12);
    read_size -= buffer.length()+1;
    temp = buffer.length(); // temp is the buffer's length
    for (long int i=0;i<temp;i++) {
      if (buffer[i] == '\0') break;
      else col_name += buffer[i];
    }

    // get and parse column data
    buffer = _fdata->read(_row_size, (char)0x12);
    read_size -= buffer.length()+1; // "+1" accounts for the deliminator (0x12) removed by read()
    temp = buffer.length();
    for (long int i=0;i<temp;i++) {
      if (buffer[i] == '\0') break;
      else col_data += buffer[i];
    }
    
    // add column name and column data pair to row
    return_row.add(col_name, col_data);

    col_name.clear();
    col_data.clear();
  }

  return return_row;
}

// edit: create empty row, write via _write_row(), then manually overwrite size field
void table::delete_row(long int row_id) {
  if (_require_optimize) {
    //_error_req_opt("delete row");
    REQ_OPT("delete row");
    if (_require_optimize) return;
  }
  if (row_id > _row_count) return;
  string delete_data(INDEX_MAX_SIZE, '0');
  for (int i=0;i<_Tstruct.size();i++) {
    delete_data.append(_Tstruct[i].name.length(), '\0');
    delete_data.append(1, (char)0x12);
    delete_data.append(_Tstruct[i].size, '\0');
    delete_data.append(1, (char)0x12);
  }
  delete_data.append(1, (char)0x11);

  _fdata->seek(_row_size*row_id);
  _fdata->write(delete_data);
  
  _deleted_row_count++;
  
  // delete key from memory (NOTE: key is not deleted from key file until close or optimization)
  for (int i=0;i<_keys.size();i++) {
    if (_keys[i].row_id == row_id) {
      _keys.erase(_keys.begin()+i);
      break;
    }
  }
}

void table::delete_row(const std::vector<row> &results) {
  for (int i=0;i<results.size();i++) delete_row(results[i].id);
}

void table::delete_row(const row& del_row) {
  if (del_row.id >= 0) delete_row(del_row.id);
  else {
    predicate p;
    for (int i=0;i<_Tstruct.size();i++) if (del_row.is_defined(_Tstruct[i])) p.And( equalto(_Tstruct[i], del_row[_Tstruct[i]]) ); // build predicate matching the row
    delete_row(select(query().where(p)));
  }
}

void table::optimize() {
  fileio* tmp_dat = new fileio;
  fileio* fileio_ptr = 0;
  
  string filename = _name;
  filename += ".tmp";
  tmp_dat->open(filename, "wb");

  _fdata->flush();
  _fdata->seek(0);

  // suspend _require_optimize:
  _require_optimize = false;

  // clear current list of key entries
  _keys.clear();

  for (long int row_id=0;row_id<_row_count;row_id++) {
    row old_row = read_row(row_id);
    if (old_row.is_empty()) continue;
    row new_row;
    
    // strip deleted columns, append new columns
    for (int i=0;i<_Tstruct.size();i++) {
      new_row.add(_Tstruct[i], old_row[_Tstruct[i].name]);
    }
    
    // write row to file
    fileio_ptr = _fdata; _fdata = tmp_dat; tmp_dat = fileio_ptr;                    // switch data pointers
    _keys.push_back( key_entry(&_key, new_row[_key.name], _fdata->pos(), row_id) ); // add key entry to keys
    _fdata->seek("END");
    _write_row(new_row._data);                                                      // write row to tmp_dat
    tmp_dat = _fdata; _fdata = fileio_ptr; fileio_ptr = 0;                          // switch dat pointers back
  }
  
  // delete *.dat, rename tmp to *.dat
  string fname = _fdata->filename();
  _fdata->rm();
  tmp_dat->mv(fname);
  delete _fdata;
  _fdata = tmp_dat;
  tmp_dat = 0;
  
  // rewrite *.key file:
  _write_keys();

  _write_structure();
  _load_structure(); // Eliminates old column aliases (alias is only set during an edit column)
  _calc_rows();
  _deleted_row_count = 0;
  _load_keys();
}

bool table::_prompt() {
  cout << "** Changing '" << _name << "' structure will require data rebuild." << endl << "   Continue? (y/n) ";
  char c;
  cin >> c;
  if (c == 'y' || c == 'Y') {
    return true;
  }
  else return false;
}

void table::_error_req_opt(string action) {  
  cout << "** Unable to " << action << "; table optimization required." << endl << "Optimize now? (y/n) ";
  char c;
  cin >> c;
  if (c == 'y' || c == 'Y') optimize();
}

void table::_load_keys() {
  _keys.clear();
  _fkeys->seek(0);

  if (_fkeys->size() > 0) {
    // load the key column into column _key:
    _key.name = _fkeys->read(-1, (char)0x12);
    _key.alias = _key.name;
    _key.size = atol(_fkeys->read(-1, (char)0x12).c_str());
    _fkeys->seek(_fkeys->pos()+1);
    while (_fkeys->pos() < _fkeys->size()) {
      key_entry temp_key_entry;
      temp_key_entry.col = &_key;
      temp_key_entry.data = _fkeys->read(-1, (char)0x12);
      temp_key_entry.fpos = atol(_fkeys->read(-1, (char)0x12).c_str());
      temp_key_entry.row_id = (temp_key_entry.fpos/_row_size);
      _keys.push_back(temp_key_entry);
      _fkeys->seek(_fkeys->pos()+1);
    }
    
  }
  else {
    _key.name = "";
    _key.alias = "";
    _key.size = 0;
  }
}

vector<row> table::select(const query &q) {
  vector<row> results;

  row temp_row;
  // perform key-search
  if (q.singularity(_key)) {
    for (int i=0;i<_keys.size();i++) {
      temp_row.add((*_keys[i].col), _keys[i].data);
      if (q.partial_eval(_key, temp_row)) {
        results.push_back(read_row(_keys[i].fpos/_row_size));
        //if (results.back().is_empty()) results.pop_back();
        if (results.back().is_empty() || !q.eval(results.back())) {
          results.erase(results.end()-1);
        }
      }
      temp_row.clear();
      if (q._limit > -1 && results.size() >= q._limit) break; //results.erase(results.end()-1);
    }
    
/*    for (int i=0;i<results.size();i++) {
      if (results[i].is_empty() || !q.eval(results[i])) {
        results.erase(results.begin()+i);
        --i;
      }
    }*/
    
    // truncate results to q._limit
    /*if (q._limit > -1) while (results.size() > q._limit) {
      if (results.size() == 0) break;
      results.erase(results.end()-1);
    }*/
  }
  // perform full search:
  else {
    for (int i=0;i<_row_count;i++) {
      results.push_back(read_row(i));
      if (results.back().is_empty() || !q.eval(results.back())) results.pop_back();
      // limit results size to q._limit
      if (results.size() >= q._limit) break;
    }
  }

  return results;
}

bool key_alpha_sort(key_entry a, key_entry b) {
  if (is_numeric(a.data) && is_numeric(b.data)) return (atoi(a.data.c_str()) < atoi(b.data.c_str()));
  a.data = strtolower(a.data);
  b.data = strtolower(b.data);

  int alength = a.data.length(), blength = b.data.length();
  for (int i=0;i<alength;i++) {
    if (a.data[i] < b.data[i]) return true;
    if (a.data[i] > b.data[i]) return false;
  }
  if (a.data.length() < b.data.length()) return true;
  else return false;
}

// Debug Functions:
void table::print_structure() {
  for (int i=0;i<_Tstruct.size();i++) {
    cout << _Tstruct[i].name << "[" << _Tstruct[i].size << "]\t";
  }
}
