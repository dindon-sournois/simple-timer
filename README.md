# Simple timer

Require MPI.  
Should automatically detect CUDA or HIP (ROCm) for optional nvtx / roctx support.

## Compiling

```bash
cmake -DCMAKE_INSTALL_PREFIX=$PWD/install -B build .
cmake --build build -j
cmake --install build
```

Basic testing:
```bash
ctest -V --test-dir build
```

## Environment variable

Ignore the first time interval measured by a timer (to hide init overhead for
example):

```bash
export SIMPLE_TIMER_SKIP_FIRST=(1|yes|y|true)
```

## Examples

[C example](main.c)

```C
#include "simple_timer.h"

tstart("foobar");
foobar();
tstop("foobar");

<...>
tprint();
```

[Fortran example](main.f90)

```Fortran
use simple_timer

call tstart("foobar")
call foobar()
call tstop("foobar")

<...>
call tprint()
```
