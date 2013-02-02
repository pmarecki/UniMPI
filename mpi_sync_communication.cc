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
  const int kSize = 1<<25;    //*4 = p_size MB
  float p_size = (float)4 * kSize / (1<<20);    //size in MB
  MPI_Status status;
  int *I, *R;
  I = new int[kSize];
  R = new int[kSize];
  printf("MPI[%d/%d] has started...\n", kTid, kDim);
  int o1 = (kTid + 1) % kDim;  	//right neighbor
  REP(i,kSize) I[i] = o1;  	//example data

  //cyclic send: 3->0, 2->3, 1->2, 0->1
  //better would be: odd>even, even>odd.
  PosixStoper xx;
  if (kTid==0) {
    //At least one thread must have receive before send, 
    //otherwise --> deadlock. 
    MPI_Recv(R, kSize, MPI_INT, kDim-1, 0, WORLD, &status); 
    MPI_Send(I, kSize, MPI_INT, 1, 0, WORLD);
  } else {
    MPI_Send(I, kSize, MPI_INT, o1, 0, WORLD);    //send is blocking here!
    MPI_Recv(R, kSize, MPI_INT, kTid-1, 0, WORLD, &status);
  }
  xx.Snap();

  if (kTid==0) printf("Problem size: %3.1fMB\n", p_size);
  printf("MPI[%d] got %d bwidth=%3.3f[MB/s]\n", kTid, R[2],
      p_size / (xx.LastDt() / 1e6));
  MPI_Finalize();
  return 0;
}
