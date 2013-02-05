#include <fstream>

#include "../STL_MACRO.h"
#include "../stoper.h"

#include <stdio.h>
#include <fcntl.h>

//compile via g++ -D_FILE_OFFSET_BITS=64 large_files.cc

//#define _LARGEFILE_SOURCE     //already defined...
//#define _LARGEFILE64_SOURCE   //already defined

//Linux 64
//(Spinner 1.4TB)
//[each:1.00MB;   files:32] time:  1572[msec], bandwidth:  21MB/s
//[each:2.00MB;   files:32] time:   316[msec], bandwidth: 212MB/s
//[each:4.00MB;   files:32] time:   751[msec], bandwidth: 179MB/s
//[each:8.00MB;   files:32] time:  1510[msec], bandwidth: 178MB/s
//[each:16.00MB;  files:32] time:  2898[msec], bandwidth: 185MB/s
//[each:32.00MB;  files:32] time:  5936[msec], bandwidth: 181MB/s
//[each:64.00MB;  files:32] time: 12247[msec], bandwidth: 175MB/s
//[each:128.00MB; files:32] time: 30179[msec], bandwidth: 142MB/s
//[each:256.00MB; files:32] time: 72889[msec], bandwidth: 118MB/s

//SSD (Intel)
//[each:1.00MB;   files:32] time:   207[msec], bandwidth: 162MB/s
//[each:2.00MB;   files:32] time:   268[msec], bandwidth: 250MB/s
//[each:4.00MB;   files:32] time:  1215[msec], bandwidth: 110MB/s
//[each:8.00MB;   files:32] time:  2753[msec], bandwidth:  98MB/s
//[each:16.00MB;  files:32] time:  6379[msec], bandwidth:  84MB/s
//[each:32.00MB;  files:32] time: 11529[msec], bandwidth:  93MB/s
//[each:64.00MB;  files:32] time: 24569[msec], bandwidth:  87MB/s
//[each:128.00MB; files:32] time: 52075[msec], bandwidth:  82MB/s
//[each:256.00MB; files:32] time:129239[msec], bandwidth:  66MB/s

/**
 *  Opening files with parameterized names:
 *
//  fstream f;
//  int k = 12;
//  char fname[8];
//  sprintf(fname,"file%4i",k);
//  f.open(fname, ios::out);
//  f.write(fname,8);
//  f.close();
//  printf(".%s.\n", fname);
 *
 */


// Main routine.
void WriteBulk(const char* fname, ulint size) {
  ulint bufor[128];
  ofstream f;
  ulint packets = size / 1024;
  PosixStoper xx;
  f.open(fname, ios::out | ios::binary);
  for(ulint i=0; i<packets; ++i) {
    f.write((char*)bufor, 1024);
  }
  f.close();
  xx.Snap();
  printf("[%4.2fMB]\ttime:%5.0f[msec],\tbandwidth:%4.0fMB/s\n",
      1. * size / 1024 / 1024, xx.LastDt()/1000, 1. / xx.LastDt() * size);
}

// Writes a number of files, each of size `ssize`.
void WriteBatches(ulint ssize, int num_files) {
  ulint bufor[128];
  char fname[12];
  ofstream f;
  ulint num_packets = ssize / 1024;
  PosixStoper xx;
  for(int j=0; j<num_files; ++j) {
    sprintf(fname,"file%3i.dat", j);
    f.open(fname, ios::out | ios::binary);
    for(ulint i=0; i<num_packets; ++i) {
      f.write((char*)&bufor, 1024);
    }
    f.close();
  }
  xx.Snap();
  printf(
      "[each:%4.2fMB; files:%i]\ttime:%5.0f[msec],\tbandwidth:%4.0fMB/s\n",
      1. *  ssize / 1024 / 1024, num_files, xx.LastDt() / 1000,
      1. / xx.LastDt() * ssize * num_files);
}

// Using file descriptors of posix interface.
// Why is this slowwwww?
void WriteBulkHuge(const char* fname, ulint size) {
  ulint num_numbers = size / 8;
  PosixStoper xx;
  int fd;
  fd = open(fname, O_RDWR);
  for(ulint i=0; i<num_numbers; ++i) {
    write(fd, (char*)&i, 8);
  }
  close(fd);
  xx.Snap();
  printf("[%4.2fMB]\ttime:%5.0f[msec],\tbandwidth:%4.0fMB/s\n",
      1. * size / 1024 / 1024, xx.LastDt()/1000, 1. / xx.LastDt() * size);
}


#include <typeinfo>

int main(void) {
//  for(int i=20; i < 29; i++)
//    WriteBatches(1 << i, 32);
//  for(int i=31; i<32; ++i)
//    WriteBulk("aa.txt", 1L<<i);


  cout << typeid(1L).name() << " " << sizeof(1UL) << endl;

}
