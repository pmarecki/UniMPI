#include "mpi.h"
#include <cstdio>
#include <cstdlib>
#include "stoper.h"         //my timing setup (will not work on Windows yet)
#include "STL_MACRO.h"      //my common macros for shorter code

#define WORLD MPI_COMM_WORLD  //everyone

int main (int argc, char *argv[])
{
  int kDim, kTid;      //after initial set, will be kept const
  MPI_Init(&argc, &argv);
  MPI_Comm_size(WORLD, &kDim);
  MPI_Comm_rank(WORLD, &kTid);
  printf("MPI[%d/%d] has started...\n", kTid, kDim);

  //// TEST OF BROADCAST (root)-->(all)  [same value]
  int common;
  if (kTid == 0) common = -1;
  MPI_Bcast(&common, 1, MPI_INT, 0, WORLD);
  printf("[%d] common=%i\n", kTid, common);  //everyone sees "common=-1"

  //// TEST OF SCATTER (root)-->(all)  [various values from `src[]`]
  int src[kDim];
  int unique;
  if (kTid == 0) REP(i,kDim) src[i]=i;
  MPI_Scatter(src, 1, MPI_INT, &unique, 1, MPI_INT, 0, WORLD);
  printf("[%i] scatter->got:%i\n", kTid, unique);    // kTid --gets--> src[kTid]

  //// TEST OF REDUCE (all)-->(root)  []
  int sum;    //for MA;
  MPI_Reduce(&unique, &sum, 1, MPI_INT, MPI_PROD, 0, WORLD);
      //also: MPI_MAX
  if (kTid==0) printf("Sum is %d should be %d\n",sum, kDim * (kDim-1)/2);
  MPI_Finalize();
  return 0;
}
