module simple_timer

  use, intrinsic :: iso_c_binding
  implicit none

  interface
     subroutine tstart_(name) bind(c, name='tstart')
       import :: c_char
       implicit none
       character(kind=c_char),dimension(*),intent(in) :: name
     end subroutine tstart_

     subroutine tstop_(name) bind(c, name='tstop')
       import :: c_char
       implicit none
       character(kind=c_char),dimension(*),intent(in) :: name
     end subroutine tstop_

     subroutine tprint_() bind(c, name='tprint')
       implicit none
     end subroutine tprint_

  end interface

  public :: tstart
  public :: tstop
  public :: tprint
  public :: tstart_
  public :: tstop_
  public :: tprint_

contains

  subroutine tstart(name)
    character(len=*),intent(in) :: name
    character(len=len_trim(name)+1) :: c_name
    c_name = trim(name) // c_null_char
    call tstart_(c_name)
  end subroutine  tstart

  subroutine tstop(name)
    character(len=*),intent(in) :: name
    character(len=len_trim(name)+1) :: c_name
    c_name = trim(name) // c_null_char
    call tstop_(c_name)
  end subroutine  tstop

  subroutine tprint()
    call tprint_()
  end subroutine  tprint


end module simple_timer
