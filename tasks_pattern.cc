/**
 * Prototype for simple distributed combinatorial search.
 *
 * Master node distributes work quanta asynchronously to workers.
 *
 * 1) In this pattern each node gets a "task" to compute, and
 *    returns bestmatch on finishing the task.
 *
 * 2) Master node distributes the tasks, and keeps track of best match.
 *
 * 3) Once the optimalmatch is found, master sends all nodes "exit" signal.
 *
 */

#include <cstdio>
#include <cstdlib>
#include "stoper.h"         //Timing classes.
#include "STL_MACRO.h"      //Common macros for shorter c++ code.

#include "MPI_macros.h"     //Our common conventions and standard tasks.


int shutdown; 

/**
 * - rething role of broadcasts: what is blocking, and what not
 * - find way to tell workers to shutdown
 * - find way for master to wait until some response received
 */ 



/////////////////////////////////////////////////
class State {
public:
  int kTid;
  int kDim;
  int kBufferSize;
  int shutdownflag_;
  int *com_buffer_;
  MPI_Status status_;

  //buffer -- raw storage for information exchange
  State(int tid, int dim, int buff_size) : kTid(tid), kDim(dim),
      kBufferSize(buff_size) {
    shutdownflag_ = 0;
    com_buffer_ = new int[buff_size];
  }
  virtual ~State() {
    delete[] com_buffer_;
  }

  virtual void Init() {}
  virtual void Shutdown() {}
  virtual void WorkLoop() {}
  virtual void BcastInt(int *k) {
    MPI_Bcast(&k, 1, MPI_INT, MASTER, WORLD);
  }
};




/////////////////////////////////////////////////

class NodeState : public State {
public:
  NodeState(int tid, int dim, int buff_size) : State(tid,dim,buff_size) {}

  virtual void Init() {
    printf("Worker node %i starting\n", kTid);
  }

  void doWork() {
    //TODO: Workers just sit idle here... when MASTER shutdowned...
    MPI_Recv(com_buffer_, kBufferSize, MPI_INT, MASTER, TAG, WORLD, &status_);
    printf("Node %i doing work->%i; sdflag=%i\n", kTid, com_buffer_[0],
        shutdown);
    com_buffer_[0] = kTid + 1000;
    MPI_Send(com_buffer_, kBufferSize, MPI_INT, MASTER, TAG, WORLD);
  }

  void WorkLoop() {
    while(!shutdown) {
      doWork();
      BcastInt(&shutdown);
    }
  }

  virtual void Shutdown() {
    BcastInt(&shutdown);
  }
};



/////////////////////////////////////////////////

class MasterState : public State {
  MPI_Request *sendrequests_;      //must be fixed containers (addresses needed)
  int startup;
public:
  //must register the nodes
  //must have communication (MPI_Request's) for nodes.
  MasterState(int ktid, int kdim, int buff_size) :
    State(ktid, kdim, buff_size) {
    sendrequests_ = new MPI_Request[kdim];  //only "0" is master
    startup = 1;
  }

  ~MasterState() {
    delete sendrequests_;
  }

  virtual void Shutdown() {
    shutdown = 1;
    BcastInt(&shutdown);  
  }

  void AssignWork() {
    //initial assign?
    for(int worker = 1; worker < kDim; ++worker) {
      //printf("testing response from node %i ...\n", worker);
      if (!startup) {
        int flag = 0;
        MPI_Test(&sendrequests_[worker], &flag, &status_);
        if (flag) {  //send new tasks, if previous finished
          printf("Master received from node %i: %i\n", worker, com_buffer_[0]);
        }
      }
      com_buffer_[0] = worker;
      MPI_Send(com_buffer_,  kBufferSize, MPI_INT, worker, TAG, WORLD);
      MPI_Irecv(com_buffer_, kBufferSize, MPI_INT, worker, TAG, WORLD,
          &sendrequests_[worker]);
    }
    startup = 0;
  }

  void WorkLoop() {
    REP(i, 20) {
      AssignWork();
      //wait for any?
      //do not destruct MPI_request...
    }
  }

};



/////////////////////////////////////////////////

int main (int argc, char *argv[])
{
  MY_MPI_INIT;
  int kBufferSize = 3;
  shutdown = 0;
  State *state;

  WORKERS_DO {
    state = new NodeState(kTid, kDim, kBufferSize);
  }
  MASTER_DO {
    state = new MasterState(kTid, kDim, kBufferSize);
  }
  state->Init();
  MPI_Barrier(WORLD);
  state->WorkLoop();
  state->Shutdown();   //do they all need to participate, or only MASTER?
  printf("Node %i exiting; sdflag=%i\n",kTid,shutdown);
  MPI_Barrier(WORLD);
  MPI_Finalize();
}
