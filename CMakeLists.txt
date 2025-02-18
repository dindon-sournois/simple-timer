cmake_minimum_required(VERSION 3.25)

project(simple_timer_lib C CXX Fortran)
set (CMAKE_CXX_STANDARD 17)

find_package(MPI REQUIRED)
find_package(CUDAToolkit)

if(DEFINED ENV{HIP_PATH})
  set(CMAKE_MODULE_PATH $ENV{HIP_PATH}/hip/cmake $ENV{HIP_PATH}/cmake)
  find_package(hip)
else()
  message("-- Could NOT find hip with HIP_PATH.")
endif()

add_library(options_flags INTERFACE)
target_compile_options(options_flags INTERFACE -O3 -Wall -g)
target_link_options(options_flags INTERFACE -O3 -Wall -g)

add_library(simple_timer SHARED simple_timer.cpp)
target_link_libraries(simple_timer PRIVATE options_flags MPI::MPI_CXX
  $<$<BOOL:${CUDAToolkit_FOUND}>:
  CUDA::nvtx3
  >
  $<$<BOOL:${hip_FOUND}>:
  hip::host
  >
)
target_compile_definitions(simple_timer PRIVATE
  $<$<BOOL:${CUDAToolkit_FOUND}>:
  CUDA
  >
  $<$<BOOL:${hip_FOUND}>:
  ROCM
  >
)

add_library(simple_timer_f SHARED simple_timer.f90)
target_link_libraries(simple_timer_f PRIVATE options_flags)

add_executable(test_c main.c)
target_link_libraries(test_c simple_timer MPI::MPI_CXX
  $<$<BOOL:${hip_FOUND}>:
  roctx64 roctracer64
  >
)

add_executable(test_f main.f90)
target_link_libraries(test_f simple_timer simple_timer_f MPI::MPI_Fortran
  $<$<BOOL:${hip_FOUND}>:
  roctx64 roctracer64
  >
)

enable_testing()
add_test(NAME test-c COMMAND mpiexec -np 4 --oversubscribe $<TARGET_FILE:test_c>)
add_test(NAME test-fortran COMMAND mpiexec -np 4 --oversubscribe $<TARGET_FILE:test_f>)
add_test(NAME test-1-rank COMMAND mpiexec -np 1 $<TARGET_FILE:test_c>)

set_target_properties(simple_timer PROPERTIES PUBLIC_HEADER "simple_timer.h")
set_target_properties(simple_timer_f PROPERTIES PUBLIC_HEADER "${CMAKE_BINARY_DIR}/simple_timer.mod")
install(TARGETS simple_timer simple_timer_f)
