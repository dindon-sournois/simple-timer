! https://stackoverflow.com/questions/6931846/sleep-in-fortran/6932232#6932232
module fortran_sleep

  use, intrinsic :: iso_c_binding, only: c_int32_t

  implicit none

  interface

     subroutine fsleep(useconds)  bind ( C, name="usleep" )
       import
       integer (c_int32_t), intent (in), VALUE :: useconds
     end subroutine fsleep

  end interface

end module fortran_sleep

program main
  use, intrinsic :: iso_c_binding, only: c_int32_t
  use mpi
  use simple_timer
  use fortran_sleep
  implicit none
  integer::ierr,rank
  integer(c_int32_t)::time

  call MPI_Init(ierr)

  call MPI_Comm_rank(MPI_COMM_WORLD, rank, ierr);

  call tstart("all ranks")
  call fsleep(int(100000/2*rank, 4))
  call tstop("all ranks")

  if (rank.eq.0) then
     call tstart("rank0")
     call fsleep(100000_4)
     call tstop("rank0")
  endif
  if (rank.eq.1) then
     call tstart("rank1___________________")
     call fsleep(100000_4)
     call tstop("rank1___________________")
  endif
  if (rank.eq.2 .or. rank.eq.1) then
     call tstart("rank1&2")
     call fsleep(int(100000*rank, 4))
     call tstop("rank1&2")
  endif

  call tprint()

  call MPI_Finalize(ierr)

end program main
