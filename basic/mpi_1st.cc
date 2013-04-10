/**
 * Step 0: compilation and start of few nodes.
 * Compile via "mpic++ mpi_1st.cc"
 * Run via "mpirun -np 12 ./a.out"
 */

#include "mpi.h"
#include <cstdio>
#include <cstdlib>

#define WORLD MPI_COMM_WORLD  //everyone


int main (int argc, char *argv[])
{
  int kDim, kTid;      //after initial set, will be kept const
  MPI_Init(&argc, &argv);
  MPI_Comm_size(WORLD, &kDim);    //total number of threads/nodes
  MPI_Comm_rank(WORLD, &kTid);    //unique identification number

  printf("MPI[%d/%d] has started...\n", kTid, kDim);

  MPI_Finalize();
}

