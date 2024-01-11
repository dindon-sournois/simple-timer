program main
  use mpi
  use simple_timer
  implicit none
  integer::ierr,rank

  call MPI_Init(ierr)

  call MPI_Comm_rank(MPI_COMM_WORLD, rank, ierr);

  call tstart("toto")
  call sleep(1)
  call tstop("toto")
  call tstart("foo")
  call sleep(1)
  call tstop("foo")
  call tstart("foo")
  call sleep(2)
  call tstop("foo")
  call tprint()

  call MPI_Finalize(ierr)

end program
