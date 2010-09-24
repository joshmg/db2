// File: fileio.h
// Written by Joshua Green

#ifndef FILEIO_H
#define FILEIO_H

#include <string>

#pragma comment(lib, "fileio.lib")

// -------------------------------------------------------------- CLASS FILEIO -------------------------------------------------------------- //
//   + open(string filename)                                                                                                                  //
//   + close()                                                                                                                                //
//   + is_open()                                                                                                                              //
//   + write(string data)                                                                                                                     //
//   + write(int data)                                                                                                                        //
//   + pos()                                                                                                                                  //
//       - returns the current file position within the file                                                                                  //
//   + seek(long long int pos)                                                                                                                //
//       - sets the current file position within the file                                                                                     //
//       - returns the current file position successfully set within the file                                                                 //
//   + seek(string "END")                                                                                                                     //
//       - sets the current file position to the end of the file                                                                              //
//       - the only acceptable value is "END"                                                                                                 //
//       - returns the current file position successfully set within the file                                                                 //
//   + read(long int length, string delim="")                                                                                                 //
//       - reads length characters from current file position or until delim is reached                                                       //
//       - advances the file position pointer                                                                                                 //
//       - if left empty, delim isn't checked                                                                                                 //
//       - delim can be a string of characters or a single character                                                                          //
//       - a length value of less than zero is replaced with the size of the file                                                             //
//   + size()                                                                                                                                 //
//       - returns the size of the file                                                                                                       //
//   + flush()                                                                                                                                //
//       - forces all modified data from buffer to the file                                                                                   //
//   + filename()                                                                                                                             //
//       - returns the name of the file                                                                                                       //
//   + rm()                                                                                                                                   //
//       - aborts any modified buffers from being written                                                                                     //
//       - closes the file                                                                                                                    //
//       - removes the file permanantly from the hard disk                                                                                    //
//   + mv(string new_name)                                                                                                                    //
//       - permanantly renames the file on the hard disk to new_name                                                                          //
//   + NOTES:                                                                                                                                 //
//       - the read and write buffer sizes can be set by modifying BUFFER_SIZE within fileio                                                  //
// ------------------------------------------------------------------------------------------------------------------------------------------ //
class fileio {
  private:
    FILE* _file;
    long long int _size;    // size of the actual file
    long long int _pointer; // the user's position in the file (extendable by buffer)
    bool _open;
    static const int BUFFER_SIZE = 1024*5;
    int _bufferfilled;          // bytes used inside _buffer
    char *_buffer;              // data queued for writing
    char *_rdbuffer;            // data queued for reading
    long long int _rdpos;       // position in file where _rdbuffer starts
    int _rdfilled;              // bytes used inside _rdbuffer
    std::string _filename;      // stored filename
  
    long long int _put(char* data, int size);
    void _refresh_size();
    void _flush();
    
    void _open_file(const std::string&, const std::string&);
    void _clear();
  
  public:
    fileio();
    ~fileio();
    bool open(std::string filename);
    bool open(std::string filename, std::string mode);
    void close();
    bool is_open();
    // Writes to a buffer until buffer overflow:
    long long int write(const std::string &data);
    long long int write(int data);
    long long int pos();
    long long int seek(long long int pos);
    long long int seek(std::string pos);
    std::string read(long int length, std::string delim="");
    std::string read(long int length, char delim);
    long long int size();
    long long int flush();
    std::string filename();
    void rm(); // Remove/Delete
    void mv(const std::string &new_name); // Move/Rename
  
    // Debug Functions:
    void file_dump();
    void fpos_dump();
    void buffer_dump();
    void data_dump();
};

// class handles deliminator checking
class delim_checker {
  private:
    std::string _delim, _matched;
    int index;
  public:
    delim_checker(const std::string &delim);
    bool found();                               // found() returns false if delim length is zero
    bool next(char c);                          // returns true if delim is matched, appends c to delim
    void clean(std::string &data);              // removes delim from data
    void reset();                               // resets delim_checker to unitialized delim
    void set(const std::string &delim);         // set the delim after a reset or empty initialization
};

#endif
