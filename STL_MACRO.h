/*
 * STL_macros.h
 *
 *  Created on: Dec 15, 2012
 *      Author: fmarecki
 */

#ifndef STL_MACROS_H_
#define STL_MACROS_H_

#include <inttypes.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <queue>
#include <stack>

using namespace std;

//Integer types
typedef uint64_t ulint;
typedef uint32_t uint;
typedef uint16_t usint;
typedef uint8_t  uchar;

//containers
typedef vector<int> vi;
typedef vector<vi> vvi;
typedef pair<int,int> ii;
typedef vector<ii> vii;
typedef set<int> si;
typedef list<int> li;

//traversing macros (use google style guide)

#define REP(i,n) for(int i=0;i<(n);++i)
#define FOR(i,a,b) for(int i=(a);i<=(b);++i)


//use `decltype` in >VC2010
#define TR(c,i) for(__typeof((c).begin()) i = (c).begin(); i != (c).end(); ++i)
#define ALL(c) (c).begin(),(c).end()



//shortcuts
#define PB push_back
#define MP make_pair
#define PRESENT(c,x) ((c).find(x) != (c).end())
#define CPRESENT(c,x) (find(ALL(c),x) != (c).end())

const int INF = 11100000;


template <typename  T>
void PrintContainer(const T w) {
  printf("{");
  TR(w, i) {
    printf("%i, ", *i);
  }
  printf("\b\b}\n");
}

void EEE() {
  printf("\n");
}


void PrintBit(uint H) {
  REP(i, 32) {
    int no = H;
    int bit = 31-i%32;
    bool val = no & (1<<bit);
    printf("%i%s%s",val,(i+1)%8==0?" ":"",(i+1)%32==0?"| ":"");
  }
  EEE();
}

uint lrot32(uint x, uint n) {
  return ((x << n) | (x >> (32-n)));
}
uint rrot32(uint x, uint n) {
  return ((x >> n) | (x << (32-n)));
}



#endif /* STL_MACROS_H_ */
