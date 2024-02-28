cmake_minimum_required(VERSION 3.25)

project(simple_timer_lib C CXX Fortran)
set (CMAKE_CXX_STANDARD 17)

find_package(MPI REQUIRED)
find_package(CUDAToolkit)

add_library(options_flags INTERFACE)
target_compile_options(options_flags INTERFACE
  -O3 -Wall -g
)
target_link_options(options_flags INTERFACE
  -O3 -Wall -g
)

add_library(simple_timer SHARED simple_timer.cpp)
target_link_libraries(simple_timer PRIVATE options_flags CUDA::nvtx3 MPI::MPI_CXX)
target_compile_definitions(simple_timer PRIVATE CUDA)

add_library(simple_timer_f SHARED simple_timer.f90)
target_link_libraries(simple_timer_f PRIVATE options_flags)

add_executable(test_c main.c)
target_link_libraries(test_c simple_timer MPI::MPI_CXX)

add_executable(test_f main.f90)
target_link_libraries(test_f simple_timer simple_timer_f MPI::MPI_Fortran)
