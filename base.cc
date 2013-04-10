/**
 * Master node distributes tasks to workers asynchronously.
 *
 *  Uses asynchronous communication. See lecture0412.pdf of ME964_2012.
 *   (also lecture0410.pdf with remark on buffers for asynchronous recv)
 */

#include <cstdio>
#include <cstdlib>
#include "stoper.h"         //Timing classes.
#include "STL_MACRO.h"      //Common macros for shorter c++ code.

#include "MPI_macros.h"     //Our common conventions and standard tasks.


#define B_SIZE    1024      //size of the communication buffor (how many int's)
#define MAX_NODES  100      //not more than this many nodes
                            //(due to buffer allocation at master)


/////////////////////////////////////////////////
// General interface of an MPI node (Master or Worker)
/////////////////////////////////////////////////
class State {
public:
  int kTid;               //node id
  int kDim;               //number of nodes
  MPI_Status status_;     //used for "MPI_Recv"s

  //buffer -- raw storage for information exchange
  State(int tid, int dim) : kTid(tid), kDim(dim) {}
  virtual ~State() {}

  virtual void Init() {}         //Run before doWork()
  virtual void doWork() {}       //Specialized by MASTER, and workers
};



/////////////////////////////////////////////////
class WorkState : public State {
  int buffer[B_SIZE];     //workers buffer
public:
  WorkState(int tid, int dim) : State(tid, dim) {}

  virtual void Init() {
    printf("Worker node %i starting\n", kTid);
  }

  // Workers do blocking "Recv" and blocking "Send"
  void doWork() {
    while(true) {
      MPI_Recv(buffer, B_SIZE, MPI_INT, MASTER, TAG, WORLD, &status_);
      if (buffer[0]== -1) break;      //flag for "shutdown"
      printf("Node %i doing work->%i.\n", kTid, buffer[0]);
      buffer[0] = kTid + 1000;
      MPI_Send(buffer, B_SIZE, MPI_INT, MASTER, TAG, WORLD);
    }
  }

};




/////////////////////////////////////////////////
class MasterState : public State {
  MPI_Request *sendrequests_;      //must be fixed containers (addresses needed)
  int startup[MAX_NODES];
  int buffers[MAX_NODES][B_SIZE];

public:
  MasterState(int ktid, int kdim) :
    State(ktid, kdim) {
    sendrequests_ = new MPI_Request[kdim];  //only "0" is master
    REP(i, kdim)
      startup[i] = 1;
  }

  ~MasterState() {
    delete sendrequests_;
  }

  virtual void Init() {
    printf("Master node %i starting\n", kTid);
  }

  //Main function repeated many times by master node.
  //Attempts to read response from each worker, analyzes it, and sends new work.
  //Case of first "Send" is special, and handled separately.
  //[This allows for sending other type of data in first "Send".]
  void AssignWork() {
    //Repeat for every worker
    for(int w = 1; w < kDim; ++w) {
      buffers[w][0] = w;
      if (!startup[w]) {
        int flag = 0;
        //Need "Irecv" before "Test"; otherwise sendrequests_==NULL --> crash
        MPI_Test(&sendrequests_[w], &flag, &status_);
        if (flag) {
          //Response received
          printf("Master received from node %i: %i\n", w, buffers[w][0]);
          // ... do some processing of the received data in buffers[w] ...
          //send [sync] some more work
          MPI_Send(buffers[w],  B_SIZE, MPI_INT, w, TAG, WORLD);
          //keep listening for results [async]
          MPI_Irecv(buffers[w], B_SIZE, MPI_INT, w, TAG, WORLD,
              &sendrequests_[w]);
        }
      } else   // Initial Send/Irecv without "MPI_Test" which would crash.
      {
        //prepare initial buffer[w] (...) then:
        MPI_Send(buffers[w],  B_SIZE, MPI_INT, w, TAG, WORLD);
        MPI_Irecv(buffers[w], B_SIZE, MPI_INT, w, TAG, WORLD,
            &sendrequests_[w]);
        startup[w] = 0;
      }
    }
  }

  //Main function for the master node.
  void doWork() {
    REP(i, 30) {
      AssignWork();
      printf("Assignment<%i>\n",i);
    }
  }

  //Sends shutdown flag to all workers.
  void callShutdown() {
    int shutdown_buffer[B_SIZE];
    shutdown_buffer[0] = -1;    //flag for shutdown
    for(int worker = 1; worker < kDim; ++worker)
      MPI_Send(shutdown_buffer, B_SIZE, MPI_INT, worker, TAG, WORLD);
  }

};



/////////////////////////////////////////////////

int main (int argc, char *argv[])
{
  MY_MPI_INIT;      //macro with usual stuff.

  State *state;

  WORKERS_DO {
    state = new WorkState(kTid, kDim);
  }
  MASTER_DO {
    printf("kDim=%i\n", kDim);
    state = new MasterState(kTid, kDim);
  }
  state->Init();
  state->doWork();
  MASTER_DO {
    ((MasterState*)state)->callShutdown();
  }
  printf("Node %i exiting...\n", kTid);
  MPI_Finalize();
}
