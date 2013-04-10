#include "mpi.h"
#include <cstdio>
#include <cstdlib>
#include "stoper.h"         //my timing setup (will not work on Windows yet)
#include "STL_MACRO.h"      //my common macros for shorter code

/**
 * Advanced communication (each talking to all others, no MASTER)
 *
 *  -> on local event (h < 123000) task sends messages to all other tasks
 *  -> these do their work, but constantly check...
 */

#define WORLD MPI_COMM_WORLD  //everyone
const int TAG = 0;
const int kMsgSize = 3;



int main (int argc, char *argv[])
{
  int kDim, kTid;      //after initial set, will be kept const
  MPI_Init(&argc, &argv);
  MPI_Comm_size(WORLD, &kDim);
  MPI_Comm_rank(WORLD, &kTid);
  MPI_Status status;
  MPI_Request *send_req, *recv_req;

  int msg[kMsgSize];       //short message sent to others on EVENT
  send_req = new MPI_Request[kDim];
  recv_req = new MPI_Request[kDim];

  uint h = 0;
  srand(time(0));

  //Initialize Async receives: from all sources
  REP(source, kDim)
    MPI_Irecv(msg, kMsgSize, MPI_INT, source, TAG, WORLD, &recv_req[source]);
  
  // Loop doing something
  REP(i,1<<16) {
    h ^= rand();
    if (h < 123000) {           //EVENT SECTION
      REP(dest, kDim) {
        int flag; 
//        if (!first_time) MPI_Wait(&send_req[i], &status); //waiting send compl
        msg[0] = 1;     //new active b-cast
        msg[1] = kTid;  //source rank
        msg[2] = h;     //for check
        //Send message to every other node (nonblocking).
        MPI_Isend(msg, 3, MPI_INT, dest, 0, WORLD, &send_req[dest]);
      }
      printf("*%d*:%d\n", kTid, h);
    }                           //END EVENT
    
    // Checking if someone broadcasted the EVENT
    REP(source, kDim) {
      int flag=0; 
      MPI_Test(&recv_req[source], &flag, &status);
      if (flag) {
        printf("MPI[%d]: info from [%d], h=%d\n", kTid, msg[1], msg[2]);
        // Receive request consumed; need to set up new one.
        MPI_Irecv(msg, 3, MPI_INT, source, 0, WORLD, recv_req+i);
      }
    }
  }
  printf("[%i]-END_BARRIER\n",kTid);
  MPI_Barrier(WORLD);   //needed for unfinished requests?
  MPI_Finalize();
  delete[] send_req; 
  delete[] recv_req; 
  return 0;
}
