/*
 * MPI_macros.h
 *
 *  Created on: Feb 8, 2013
 *      Author: fmarecki
 */

#ifndef MPI_MACROS_H_
#define MPI_MACROS_H_

#include "mpi.h"


#define WORLD MPI_COMM_WORLD  //everyone
const int TAG = 0;
#define MASTER 0
#define MASTER_DO  if (kTid==MASTER)
#define WORKERS_DO if (kTid!=MASTER)
#define WORLD  MPI_COMM_WORLD

#define MY_MPI_INIT           \
  int kDim, kTid;             \
  MPI_Init(&argc, &argv);     \
  MPI_Comm_size(WORLD, &kDim);\
  MPI_Comm_rank(WORLD, &kTid);\
  MPI_Status STATUS



#endif /* MPI_MACROS_H_ */
