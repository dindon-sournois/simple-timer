# Simple timer

Require MPI.  
Should optionally detect CUDA or HIP (ROCm) automatically for nvtx / roctx
support.

## Compiling

```bash
cmake -B build .
cmake --build build -j
ctest -V --test-dir build
```

## Examples

[C example](main.c)

[Fortran example](main.f90)
