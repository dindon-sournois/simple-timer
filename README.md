# Simple timer

Require MPI.  
Should automatically detect CUDA or HIP (ROCm) for optional nvtx / roctx support.

## Compiling

```bash
cmake -DCMAKE_INSTALL_PREFIX=$PWD/install -B build .
cmake --build build -j
cmake --install build
ctest -V --test-dir build
```

## Examples

[C example](main.c)

[Fortran example](main.f90)
