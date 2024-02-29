#include <unistd.h>
#include <mpi.h>

#include "simple_timer.h"

int main(){
  MPI_Init(NULL,NULL);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  tstart("all ranks");
  usleep(100000*rank*0.5);
  tstop("all ranks");

  if (rank==0){
    tstart("rank0");
    usleep(100000);
    tstop("rank0");
  }
  if (rank==1){
    tstart("rank1___________________");
    usleep(100000);
    tstop("rank1___________________");
  }

  if (rank==2 || rank==1){
    tstart("rank1&2");
    usleep(100000*rank);
    tstop("rank1&2");
  }

  tprint();

  MPI_Finalize();
}
