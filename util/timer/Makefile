all:

rocm:
	$(eval ROCM=-L${ROCM_PATH}/lib -lroctx64 -lroctracer64 -I${ROCM_PATH}/include)
cuda:
	$(eval CUDA=-gpu=cc80,cuda12.2 -cuda -lnvToolsExt)

c:
	mpic++ -g -O3 -fpic -std=c++17 -Wall simple_timer.cpp -shared $(CUDA) $(ROCM) -c
	mpic++ -g -O3 -fpic -std=c++17 -Wall simple_timer.cpp -shared -o libsimple_timer.so $(CUDA) $(ROCM)
fortran: c
	mpif90 -g -O3 -fpic -Wall simple_timer.f90 -shared $(CUDA) $(ROCM) -c -o simple_timer_f.o
	mpif90 -g -O3 -fpic -Wall simple_timer.f90 -shared -o libsimple_timer_f.so $(CUDA) $(ROCM)

cuda-c: cuda c
rocm-c: rocm c
cuda-fortran: cuda fortran
rocm-fortran: rocm fortran

c-test: c
	mpicc -g -O3 -Wall main.c -L. -lsimple_timer -o main_c $(CUDA) $(ROCM)
fortran-test: fortran
	mpif90 -g -O3 -Wall main.f90 -L. -lsimple_timer_f -lsimple_timer -o main_f90 $(CUDA) $(ROCM)

cuda-fortran-test: cuda-fortran fortran-test
rocm-fortran-test: rocm-fortran fortran-test
cuda-c-test: cuda-c c-test
rocm-c-test: rocm-c c-test

test: c-test fortran-test 
	mpirun -n 2 ./main_c
	mpirun -n 2 ./main_f90
test-cuda: cuda-c-test cuda-fortran-test 
test-rocm: rocm-c-test rocm-fortran-test

clean:
	rm -f *.o *.so a.out *.mod main_c main_f90
