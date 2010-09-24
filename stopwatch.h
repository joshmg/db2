// File: stopwatch.h
// Written by Joshua Green

#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <cstdio>
#include <ctime>

class stopwatch {
  private:
    time_t ti, tf;
    
  public:
    void start() {
      ti = clock();
    }
    void stop() {
      tf = clock();
    }
    void clear() {
      ti=0;
      tf=0;
    }
    void print() {
      printf("    [Elapsed Time: ~%.3lf seconds.]\n", (tf-ti)/(double)CLOCKS_PER_SEC);
    }
};

#endif
