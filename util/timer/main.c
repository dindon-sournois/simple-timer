#include <unistd.h>
#include <mpi.h>

#include "simple_timer.h"

int main(){
  MPI_Init(NULL,NULL);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  tstart("toto");
  usleep(1000000);
  tstop("toto");
  tstart("foo");
  usleep(1000000);
  tstop("foo");

  if (rank==0){
    tstart("rank0");
    usleep(10000);
    tstop("rank0");
  }
  if (rank==1){
    tstart("rank1___________________");
    usleep(10000);
    tstop("rank1___________________");
  }

  tprint();

  MPI_Finalize();
}
