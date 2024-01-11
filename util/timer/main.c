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
  tstart("toto");
  usleep(2000000);
  tstop("toto");
  tstart("deadbeef1");
  usleep(10000);
  tstop("deadbeef1");
  tstart("deadbeef2");
  usleep(10000);
  tstop("deadbeef2");
  tstart("deadbeef2");
  usleep(10000);
  tstop("deadbeef2");
  tstart("deadbeef3");
  usleep(10000);
  tstop("deadbeef3");
  tstart("deadbeef4");
  usleep(10000);
  tstop("deadbeef4");
  tstart("deadbeef5");
  usleep(10000);
  tstop("deadbeef5");
  tstart("deadbeef6");
  usleep(10000);
  tstop("deadbeef6");
  tstart("deadbeef7");
  usleep(10000);
  tstop("deadbeef7");

  if (rank==0){
    tstart("rank0");
    usleep(10000);
    tstop("rank0");
  }
  if (rank==1){
    tstart("rank1");
    usleep(10000);
    tstop("rank1");
  }

  tprint();

  MPI_Finalize();
}
