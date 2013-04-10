/**
 * Prototype for simple distributed combinatorial search.
 * Each node searches for best bit-match (most bits matched) of
 * a given number KEY;
 *
 * To be run on 8 machines:
 *  "mpirun -np 8 ./a.out"
 *
 * Note: basic and derived datatypes:
 *  --> http://beige.ucs.indiana.edu/I590/node100.html
 */

#include "mpi.h"
#include <cstdio>
#include <cstdlib>
#include "stoper.h"         //my timing setup (will not work on Windows yet)
#include "STL_MACRO.h"      //my common macros for shorter code

#define WORLD MPI_COMM_WORLD  //everyone
#define MASTER 0
#define MASTER_DO  if (kTid==MASTER)
#define WORKERS_DO if (kTid!=MASTER)


const int local_bits = 21;          //Loop over 2^local_bits at each node.
const int unique_bits = 3;          //First 2^unique_bits = node number = kTid.

/**
 * Verifies how many bits of "key" and tested number "x" are matched.
 * Searches only through accessible scope of bit=0..(local+unique)bits.
 */
int MatchedBits(ulint key, ulint x) {
  int ans = 0;
  REP(i, local_bits + unique_bits) {
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

  ///// I) Problem initialization stage.
  //   MASTER sends the "KEY" to WORLD.
  uint64_t key;
  MASTER_DO key = 123456789123456789;
  MPI_Bcast(&key, 1, MPI_LONG_INT, MASTER, WORLD);
  printf("[%i] common=%li\n", kTid, key);  //everyone reports correct KEY

  ///// II) Work-assignment stage.
  int first3bits = 0;   //Will be different for each node.
  MASTER_DO {
    for(int dest_node=1; dest_node<8; ++dest_node)
      MPI_Send(&dest_node, 1, MPI_INT, dest_node, 0, WORLD);
  }
  WORKERS_DO {
    MPI_Recv(&first3bits, 1, MPI_INT, MASTER, 0, WORLD, &status);
  }

  ///// III) Work-execution stage.
  //  Checking lowest "local_bits",
  //   while "unique_bits = first3bits" assigned in II.
  ulint base = first3bits << local_bits;
  int max = 0;
  ulint bestmatch;
  REP(i, 1<<local_bits) {
    ulint test = base + i;
    if (MatchedBits(key, test) > max) {
      max = MatchedBits(key, test);
      bestmatch = test;
    }
  }
  printf("Locally best match @[%i]: %i outof 64 bits\n", kTid, max);


  ///// IV) Global reduction stage
  /////    Part A: localization of solution.
  // REDUCE (WORLD) --> (root)
  // Here: with MPI_MAXLOC, finding the node with maximum value.
  uint tosend[2];
  tosend[0] = max;
  tosend[1] = kTid;
  uint global_best[2];    //result storage at MASTER
  MPI_Reduce(tosend, global_best, 1, MPI_2INT, MPI_MAXLOC, MASTER, WORLD);
  //MPI_Reduce with MPI_MINLOC compares on tosend[0], but copies also tosend[1]
  // (note MPI_2INT) to "global_best[]".
  int bestnode = global_best[1];
  MASTER_DO {
    printf("Globally best match: %i outof 64 bits, ", (int)global_best[0]);
    printf("(by node %i)\n", global_best[1]);
  }

  /////  Part IVB: retrieving full data from best node.
  //Get bestmatch from bestnode.
  int com_bestnode = bestnode;   //Communicate to WORLD who bestnode was.
  MPI_Bcast(&com_bestnode, 1, MPI_INT, MASTER, WORLD);

  ulint com_bestmatch;   //Only bestnode sends its bestmatch.
  if (kTid == com_bestnode) {
    MPI_Send(&bestmatch, 1, MPI_LONG_INT, MASTER, 0, WORLD);
    printf("Bestnode (%i): sending bestmatch:%li\n", kTid, bestmatch);
  }
  if (kTid == MASTER) {           //Master displays bestmatch.
    MPI_Recv(&com_bestmatch, 1, MPI_LONG_INT, bestnode, 0, WORLD, &status);
    printf("Master obtained bestmatch=%li\n", com_bestmatch);
    printf("KEY = "); PrintBit(key); printf("\n");
    printf("BEST= "); PrintBit(com_bestmatch); printf("\n");
  }

  MPI_Finalize();
  return 0;
}
