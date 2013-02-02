#include "mpi.h"
#include <cstdio>
#include <cstdlib>
#include "stoper.h"         //my timing setup (will not work on Windows yet)
#include "STL_MACRO.h"      //my common macros for shorter code

#define WORLD MPI_COMM_WORLD  //everyone

int main (int argc, char *argv[])
{
  MPI_Status status;
  int kDim, kTid;      //after initial set, will be kept const
  MPI_Init(&argc, &argv);
  MPI_Comm_size(WORLD, &kDim);
  MPI_Comm_rank(WORLD, &kTid);

  int *m = new int[3];       //short message
  MPI_Request s, r;

  printf("MPI[%d/%d] has started...\n", kTid, kDim);
  uint h;
  MPI_Barrier(WORLD);
  
  int flag;
  MPI_Test(&r, &flag, &status); //returns immediately, if Request not registerd

  //Initialize Async receive
  MPI_Irecv(m, 3, MPI_INT, MPI_ANY_SOURCE,  0, WORLD, &r);
  MPI_Isend(m, 3, MPI_INT, (kTid+1) % kDim, 0, WORLD, &s);
  printf("[%d]:after async calls.\n",kTid);
  int count=0;
  int fr=1,fs=1; 
  REP(i, 1<<10) {
    count++;
    int flag; 
    if (count % 128 == 0) {
      MPI_Test(&r, &flag, &status);
      if (flag && fr) {
        printf("[%d] recv_OK\n",kTid);
        fr=0;     	//do not repeat the communicate
      }
      MPI_Test(&s, &flag, &status);
      if (flag && fs) {
        printf("[%d] send_OK\n",kTid);
        fs=0;     	//do not repeat the communicate
      }
    }
    if (count % 32 == 0) printf("<[%d]%d>", kTid, count);
  }
  MPI_Barrier(WORLD);
  MPI_Finalize();
  delete[] m;
  return 0;
}
