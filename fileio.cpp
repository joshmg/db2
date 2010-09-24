// File: fileio.cpp
// Written by Joshua Green

#include <iostream>

#include "fileio.h"
#include <string>
#include <cstdio>
#include <iostream>
using namespace std;

fileio::fileio() {
  _buffer = 0;
  _rdbuffer = 0;
  _file = 0;

  _clear();
}

fileio::~fileio() {
  close();
}

void fileio::_clear() {
    if (_buffer != 0) delete[] _buffer;
    if (_rdbuffer != 0) delete[] _rdbuffer;
    if (_file != 0) fclose(_file);

    _size = 0;
    _pointer = 0;
    _open = false;
    _bufferfilled = 0;
    _rdpos = 0;
    _rdfilled = 0;
    _filename.clear();
}

// default behavior: read binary, if failed, write binary
bool fileio::open(string filename) {
  if (_open) return false;
  _open_file(filename, "rb+");
  return _open;
}

bool fileio::open(string filename, string mode) {
  if (_open) return false;
  
  if (mode == "r") mode = "rb";
  else if (mode == "r+" || mode == "rw") mode = "rb+";
  else if (mode == "w") mode = "wb";  
  else if (mode == "w+") mode = "wb+";
  
  if (mode[1] != 'b') return false;
  
  _open_file(filename, mode);
  return _open;
}

// attempts to open with supplied mode, upon failure attempts to open with wb+
void fileio::_open_file(const string &filename, const string &mode) {
  _file = fopen(filename.c_str(), mode.c_str());
  if (_file != 0) _open = true;
  /*else {
    _file = fopen(filename.c_str(), "wb+");
    if (_file != 0) _open = true;
    else _open = false;
  }*/
  if (_open) {
    _filename = filename;
    _buffer = new char[BUFFER_SIZE+1];
    _rdbuffer = new char[BUFFER_SIZE+1];
    for (int i=0;i<BUFFER_SIZE;i++) {
      _buffer[i] = '\0';
      _rdbuffer[i] = '\0';
    }
    fseek(_file, 0, SEEK_SET);
    _refresh_size();
    _pointer = 0;
    clearerr(_file);
  }
}

void fileio::close() {
  if (_open) {
    // dump buffer before closing...
    if (_bufferfilled > 0) {
      fseek(_file, 0, SEEK_END);
      _put(_buffer, _bufferfilled);
    }
    fclose(_file);
    _file = 0;
    
    _clear();
  }
}

bool fileio::is_open() {
  return _open;
}

long long int fileio::write(const string &data) {
  if (!is_open()) return 0;
  
  long long int length = data.size();
  long long int stored = 0;
  
  // maintain read buffer:
  if ((_rdpos > -1) && _pointer >= _rdpos && _pointer < _rdpos+BUFFER_SIZE && _rdfilled > 0) {
    for (long int i=0;i<BUFFER_SIZE;i++) {
      if (i >= data.length() || (i+(_pointer-_rdpos) >= BUFFER_SIZE)) break;
      _rdbuffer[i+(_pointer-_rdpos)] = data[i];
      // maintain _rdfilled (the amount of read buffer that is filled)
      if (i+(_pointer-_rdpos) >= _rdfilled) _rdfilled++;
    }
  }
  
  // if overwriting data:
  if (_pointer < _size+_bufferfilled) {
    // if overwriting file:
    if (_pointer < _size) {
      fseek(_file, _pointer, SEEK_SET); // Ensure the file is writing to the place the user thinks it is
      long long int bytes_into_file = _size-_pointer;
      if (bytes_into_file > length) bytes_into_file = length;
      char *c_data = new char[bytes_into_file+1];
      for (int i=0;i<bytes_into_file;i++) {
        c_data[i] = data[i];
      }
      c_data[bytes_into_file] = '\0';
      long long int written;
      written = _put(c_data, bytes_into_file);
      stored += written;
      _pointer += written;
      _refresh_size();      
      delete[] c_data;
    }
    // if overwriting buffer:
    if (stored < length) {
      for (int i=(_pointer-_size);i<_bufferfilled;i++) {
        if (stored >= length) return stored;
        _buffer[i] = data[stored];
        stored++;
        _pointer++;
      }
    }
  }
  // if appending to buffer:
  if (_pointer >= _size+_bufferfilled && stored < length) {
    while (stored < length) {
      // write buffer to file if buffer overflowed:
      if (_bufferfilled >= BUFFER_SIZE) {
        _put(_buffer, _bufferfilled);
        _bufferfilled = 0;
      }

      // store new data into buffer:
      _buffer[_bufferfilled] = data[stored];
      _bufferfilled++;

      // update file pointer:
      _pointer++;

      stored++;
    }
  }
  return stored;
}

long long int fileio::write(int data) {
  string temp;
  temp += data;
  return write(temp);
}

// _put()
// Doesn't move _pointer
// Moves the actual file pointer to the end of the written data
// Recalculates _size
long long int fileio::_put(char *data, int size) {
  if (_file == 0) return 0;
  clearerr(_file);
  _flush();
  int written = fwrite(data, sizeof(char), size, _file);
  _flush();

  // Recalculate _size
  _refresh_size();

  return written;
}

long long int fileio::pos() {
  return _pointer;
}

long long int fileio::seek(long long int fpos) {
  if (!is_open()) return 0;
  
  if (fpos < 0) fpos = 0;
  clearerr(_file);
  // Allow pointer to access position in buffer:
  if (fpos >= _size) {
    if (fpos > _bufferfilled+_size) fpos = _bufferfilled+_size;
    _pointer = fpos;
    fseek(_file, _size, SEEK_SET);
  }
  else {
    fseek(_file, fpos, SEEK_SET);
    _pointer = ftell(_file);
  }
  clearerr(_file);

  return _pointer;
}

long long int fileio::seek(string fpos) {
  if (!is_open()) return 0;
  
  clearerr(_file);
  if (fpos == "END" || fpos == "end") {
    _pointer = _size+_bufferfilled;
    fseek(_file, 0, SEEK_END);
  }
  clearerr(_file);
  return _pointer;
}

string fileio::read(long int length, char delim) {
  return read(length, string(1, delim));
}

string fileio::read(long int length, string delim) {
  if (!is_open()) return "";
  
  if (length < 0) length = size();
  long long int read_length = 0;
  string data;
  char c;

  if (_pointer < _size) fseek(_file, _pointer, SEEK_SET); // ensure proper internal file pointer
  clearerr(_file);
  
  // deliminator variable:
  delim_checker deliminator(delim);

  // load and maintain read buffer
  // if pointer is less than currently loaded read buffer pos
  //   and pointer is not reading from the write buffer
  if ((_pointer < _rdpos || _pointer > _rdpos+_rdfilled || _rdpos < 0) && _pointer < _size) {
    _rdpos = ftell(_file);
    _rdfilled=0;
    for (int i=0;i<BUFFER_SIZE && i<_size-_pointer;i++) {
      _rdbuffer[i] = (char)getc(_file);
      _rdfilled++;
    }
  }

  // Read from read buffer
  if (_pointer >= _rdpos && _pointer < _rdpos+_rdfilled) {
    for (long long int i=0;i<BUFFER_SIZE;i++) {
      if (_pointer-_rdpos >= _rdfilled) break;

      data.push_back(_rdbuffer[_pointer-_rdpos]);
      read_length++;
      _pointer++;
      
      // delim check:
      if (deliminator.next(_rdbuffer[(_pointer-1)-_rdpos])) {
        deliminator.clean(data);
        return data;
      }
      
      if (read_length >= length) return data;
    }
  }

  // Read from file:
  if (_pointer < _size) fseek(_file, _pointer, SEEK_SET); // ensure proper internal file pointer
  clearerr(_file);
  _flush();
  while (_pointer < _size) {
    c = (char)getc(_file);
    data.push_back(c);
    _pointer++;
    read_length++;

    // delim check:
    if (deliminator.next(c)) {
      deliminator.clean(data);
      return data;
    }
    
    if (read_length >= length) return data;
  }
  
  // Read from write buffer:
  while (_pointer < _size+_bufferfilled && _pointer-_size >= 0) {
    data.push_back(_buffer[_pointer-_size]);
    _pointer++;
    read_length++;
    
    // delim check:
    if (deliminator.next(_buffer[(_pointer-1)-_size])) {
      deliminator.clean(data);
      return data;
    }
    
    if (read_length >= length) return data;
  }
  _flush();
  return data;
}

long long int fileio::size() {
  if (!is_open()) return 0;
  
  _refresh_size();  
  return _size+_bufferfilled;
}

long long int fileio::flush() {
  if (!is_open()) return 0;
  
  long long int written;
  written = _put(_buffer, _bufferfilled);
  _bufferfilled = 0;
  return written;
}

void fileio::_refresh_size() {
  long long int pos = ftell(_file);
  fseek(_file, 0, SEEK_END);
  _size = ftell(_file);
  fseek(_file, pos, SEEK_SET);  
}

void fileio::_flush() {
  long long int fpos = ftell(_file);
  fflush(_file);
  fseek(_file, fpos, SEEK_SET);
}

string fileio::filename() {
  return _filename;
}

void fileio::rm() {
  string filename = _filename;
  _bufferfilled = 0;
  close();
  remove(filename.c_str());
}

void fileio::mv(const string &new_name) {
  string filename = _filename;
  close();
  rename(filename.c_str(), new_name.c_str());
  open(new_name);
}

// -----------------------------------------------------------
// Deliminator Checker Class:
// -----------------------------------------------------------

delim_checker::delim_checker(const string &delim) {
  set(delim);
}

bool delim_checker::found() {
  return ((_delim == _matched) && (_delim.length() > 0));
}

bool delim_checker::next(char c) {
  if (index >= _delim.length()) return found();
  
  if (_delim[index] == c) {
    _matched.append(1, c);
    index++;
  }
  else {
    index = 0;
    _matched.clear();
  }
  return found();
}

void delim_checker::clean(string &data) {
  if (found()) data.erase(data.length()-_delim.length(), _delim.length());
}

void delim_checker::reset() {
  _delim.clear();
  _matched.clear();
  index = 0;
}

void delim_checker::set(const string &delim) {
  reset();
  _delim = delim;
}

// -----------------------------------------------------------
// Debug Functions:
// -----------------------------------------------------------
void fileio::file_dump() {
  string data;
  long long int pos = ftell(_file),
                    size =0;
  fseek(_file, 0, SEEK_END);
  size = ftell(_file);
  fseek(_file, 0, SEEK_SET);
  char *c_data = new char[size+1];
  fread(c_data, sizeof(char), size, _file);
  for (int i=0;i<size;i++) {
    data.append(1, c_data[i]);
  }
  delete[] c_data;
  fseek(_file, pos, SEEK_SET);

  cout << "-------FILE-------" << endl;
  cout << "|" << data << "|" << endl;
  cout << "------------------" << endl;
}

void fileio::fpos_dump() {
  cout << "-------FPOS-------" << endl;
  cout << ftell(_file) << endl;
  cout << "------------------" << endl;
}

void fileio::buffer_dump() {

  cout << "-------BUFF-------" << endl;
  cout << "|";
  for (int i=0;i<_bufferfilled;i++) cout << _buffer[i];
  if (_bufferfilled+1 < BUFFER_SIZE) cout << "[" << _buffer[_bufferfilled+1] << "]";
  if (_bufferfilled+2 < BUFFER_SIZE) cout << "[" << _buffer[_bufferfilled+2] << "]...";
  cout << "|" << endl;
  cout << "------RDBUFF------" << endl;
  cout << "|";
  for (int i=0;i<_rdfilled;i++) cout << _rdbuffer[i];
  if (_rdfilled+1 < BUFFER_SIZE) cout << "[" << _rdbuffer[_rdfilled+1] << "]";
  if (_rdfilled+2 < BUFFER_SIZE) cout << "[" << _rdbuffer[_rdfilled+2] << "]...";
  cout << "|" << endl;
  cout << "------------------" << endl;
}

void fileio::data_dump() {
  cout << "-------DATA------" << endl;
  cout << "_size = " << _size << endl;
  cout << "_pointer = " << _pointer << endl;
  cout << "_open = " << _open << endl;
  cout << "BUFFER_SIZE = " << BUFFER_SIZE << endl;
  cout << "_bufferfilled = " << _bufferfilled << endl;
  cout << "_rdpos = " << _rdpos << endl;
  cout << "_rdfilled = " << _rdfilled << endl;
  cout << "------------------" << endl;
}
// -----------------------------------------------------------
