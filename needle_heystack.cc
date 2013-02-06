/**
 * Prototype for simple distributed combinatorial search.
 * Each node searches for best bit-match (most bits matched) of
 * a given number KEY;
 *
 * To be run on 8 machines:
 *  "mpirun -np 8 ./a.out"
 *
 */

#include "mpi.h"
#include <cstdio>
#include <cstdlib>
#include "stoper.h"         //my timing setup (will not work on Windows yet)
#include "STL_MACRO.h"      //my common macros for shorter code

#define WORLD MPI_COMM_WORLD  //everyone
#define MASTER 0


// How many bits of "key" and tested number "x" are matched.
int MatchedBits(ulint key, ulint x) {
  int ans = 0;
  REP(i,64) {
    if ((key&(1<<i)) == (x&(1<<i))) ++ans;
  }
  return ans;
}


int main (int argc, char *argv[])
{
  int kDim, kTid;      //after initial set, will be kept const
  MPI_Init(&argc, &argv);
  MPI_Comm_size(WORLD, &kDim);
  MPI_Comm_rank(WORLD, &kTid);
  printf("MPI[%d/%d] has started...\n", kTid, kDim);
  MPI_Status status;

  //Send the "KEY" from MASTER to everyone in WORLD.
  uint64_t KEY;
  if (kTid == 0) KEY = 123456789123456789;
  MPI_Bcast(&KEY, 1, MPI_LONG_INT, 0, WORLD);
  printf("[%i] common=%li\n", kTid, KEY);  //everyone sees "common=-1"

  int first3bits;   //Will be different for each node.
  if (kTid == 0) {
    //send
    for(int i=1; i<8; ++i)
      MPI_Send(&i, 1, MPI_INT, i, 0, WORLD);
    //master's share of work...
    first3bits = 0;
  } else {
    MPI_Recv(&first3bits, 1, MPI_INT, 0, 0, WORLD, &status);
  }

  //Compute part -- check lowest 20 bits, with upper 3 from "first3bits"
  ulint base = first3bits << 20;
  int max = 0;
  ulint bestmatch;
  REP(i,1<<20) {
    ulint test = base + i;
    if (MatchedBits(KEY, test) > max) {
      max = MatchedBits(KEY, test);
      bestmatch = test;
    }
  }
  printf("Locally best match @[%i]: %i outof 24 bits\n", kTid, max);

  //// TEST OF REDUCE (all)-->(root)  []
  int global_best;    //for MASTER;
  MPI_Reduce(&max, &global_best, 1, MPI_INT, MPI_MAX, 0, WORLD);
  MPI_Barrier(WORLD);

  if (kTid==0)
    printf("Globally best match: %i outof 64 bits\n", global_best);

  MPI_Finalize();
  return 0;
}
