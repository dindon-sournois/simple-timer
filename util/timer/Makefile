#CUDA_ROOT=$(dirname $(which nvcc))/..
#CUDA_INC=-I$(CUDA_ROOT)/include
#CUDA_LIB=-L$(CUDA_ROOT)/lib64/stubs -lcuda
CUDA=-gpu=cc80,cuda12.2 -cuda -lnvToolsExt
ROCM=-L${ROCM_PATH}/lib -lroctx64 -lroctracer64 -I${ROCM_PATH}/include

all: 

fortran: c
	mpif90 -g -O3 -fpic -Wall simple_timer.f90 -shared -o libsimple_timer_f.so

cuda-fortran: cuda-c
	mpif90 -g -O3 -fpic -Wall simple_timer.f90 -shared -o libsimple_timer_f.so $(CUDA)

rocm-fortran: rocm-c
	mpif90 -g -O3 -fpic -Wall simple_timer.f90 -shared -o libsimple_timer_f.so $(ROCM)

fortran-test: fortran
	mpif90 -g -O3 -Wall main.f90 -L. -lsimple_timer_f -lsimple_timer -o main_f90

cuda-fortran-test: cuda-fortran
	mpif90 -g -O3 -Wall main.f90 -L. -lsimple_timer_f -lsimple_timer -o main_f90 $(CUDA)

rocm-fortran-test: rocm-fortran
	mpif90 -g -O3 -Wall main.f90 -L. -lsimple_timer_f -lsimple_timer -o main_f90 $(ROCM)

c:
	mpic++ -g -O3 -fpic -std=c++17 -Wall simple_timer.cpp -shared -o libsimple_timer.so

cuda-c:
	mpic++ -g -O3 -fpic -std=c++17 -Wall simple_timer.cpp -shared -o libsimple_timer.so $(CUDA)

rocm-c:
	mpic++ -g -O3 -fpic -std=c++17 -Wall simple_timer.cpp -shared -o libsimple_timer.so $(ROCM)

c-test: c
	mpicc -g -O3 -Wall main.c -L. -lsimple_timer -o main_c

cuda-c-test: cuda-c
	mpicc -g -O3 -Wall main.c -L. -lsimple_timer -o main_c $(CUDA)

rocm-c-test: rocm-c
	mpicc -g -O3 -Wall main.c -L. -lsimple_timer -o main_c $(ROCM)

test: c-test fortran-test 
test-cuda: cuda-c-test cuda-fortran-test 
test-rocm: rocm-c-test rocm-fortran-test

clean:
	rm -f *.o *.so a.out *.mod main_c main_f90
