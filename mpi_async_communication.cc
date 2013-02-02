#include "mpi.h"
#include <cstdio>
#include <cstdlib>
#include "stoper.h"         //my timing setup (will not work on Windows yet)
#include "STL_MACRO.h"      //my common macros for shorter code

/**
 * Advanced communication:
 *  -> on event (h < 123000) task sends messages to all other tasks
 *  -> these do their work, but constantly check...
 */

#define WORLD MPI_COMM_WORLD  //everyone

int main (int argc, char *argv[])
{
  MPI_Status status;
  MPI_Request *s, *r;
  int kDim, kTid;      //after initial set, will be kept const
  MPI_Init(&argc, &argv);
  MPI_Comm_size(WORLD, &kDim);
  MPI_Comm_rank(WORLD, &kTid);

  int m[3];       //short message
  s = new MPI_Request[kDim];
  r = new MPI_Request[kDim];

  printf("MPI[%d/%d] has started...\n", kTid, kDim);
  MPI_Barrier(WORLD);
  uint h;
  srand(time(0) + kTid);  //unique randomizer for each thread

  //Initialize Async receives: from all sources
  REP(i, kDim) MPI_Irecv(m, 3, MPI_INT, i, 0, WORLD, r+i);
  
  // Loop doing
  REP(i,1<<16) {
    h ^= rand();
    if (h < 123000) {      //bcast section
      REP(i, kDim) {
        int flag; 
//        if (!first_time) MPI_Wait(s+i, &status);
        m[0] = 1;     //new active b-cast
        m[1] = kTid;  //source rank
        m[2] = h;     //for check
        MPI_Isend(m, 3, MPI_INT, i, 0, WORLD, s+i);  //send to all
//        first_time=0;
      }
      printf("*%d*:%d\n",kTid, h);
    }//end send; 
    
    REP(i, kDim) {    //getting info from `rank=i`
      int flag=0; 
      MPI_Test(r+i, &flag, &status);      //recv_request r[i]
      if (flag) {
        //printout of received information
        printf("MPI[%d]: info from [%d], h=%d\n", kTid, m[1], m[2]);
        MPI_Irecv(m, 3, MPI_INT, i, 0, WORLD, r+i); //form new request 
      }
    }
  }
  printf("[%d]-END_BARRIER\n",kTid);
  MPI_Barrier(WORLD);
  MPI_Finalize();
  delete[] s; 
  delete[] r; 
  return 0;
}
