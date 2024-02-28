#include <mpi.h>
#include <iostream>
#include <chrono>
#include <map>
#include <algorithm>

#ifdef _CUDA
#include "nvToolsExt.h"
#elif _HIP
#include <roctracer/roctx.h>
#endif

#include "simple_timer.h"

typedef struct {
  std::chrono::steady_clock::time_point begin;
  int color;
  double total; // ns
} entry;

std::map<std::string, entry> timers;

#ifdef _CUDA
const uint32_t colors[] = { 0xf24826, 0xd17c3c, 0xd3b632, 0x31c692, 0x4123af,
                            0x7c51bc, 0xc317ea, 0xc11b3f, 0xbf931c, 0xffe949,
                            0x6ffcf7, 0x38a8b5, 0x5a1fad, 0xf268ad, 0xfc2376,
                            0xb55d0a, 0xceb71e, 0x31cc02, 0x0f8c88, 0x996bf4,
                            0xe01dcc, 0xcc4a22, 0xcc963b, 0xfcef3c, 0x71e537,
                            0x6f7af7, 0x9644bf, 0xe55bcc, 0xea3c02, 0xc99f34,
                            0xeff268, 0x95ea0b, 0x7845ef, 0x9404f4, 0xe85ad7 };
const int num_colors = sizeof(colors)/sizeof(uint32_t);
int color_counter = 0;
#endif

void tstart(const char* name){
  std::string s = std::string(name);
  auto now = std::chrono::steady_clock::now();
  // init total to 0 at first insertion
  auto ret = timers.try_emplace(s,entry{now,0,0.0});
  bool new_entry = ret.second;
  if(new_entry) {
#ifdef _CUDA
    timers[s].color = (color_counter++)%num_colors;
#endif
  } else {
    timers[s].begin = now;
  }
#ifdef _CUDA
  int color_id = timers[s].color;
  nvtxEventAttributes_t eventAttrib = {0};
  eventAttrib.version = NVTX_VERSION;
  eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
  eventAttrib.colorType = NVTX_COLOR_ARGB;
  eventAttrib.color = colors[color_id];
  eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
  eventAttrib.message.ascii = name;
  nvtxRangePushEx(&eventAttrib);
#elif _HIP
  roctxRangePush(name);
#endif
}

void tstop(const char* name){
  auto now = std::chrono::steady_clock::now();
  auto diff = now - timers[name].begin;
  double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
  timers[name].total += duration;
#ifdef _CUDA
  nvtxRangePop();
#elif _HIP
  roctxRangePop();
#endif
}

inline double sec(double nano){
  return nano / 1000000000.0;
}

void min_func(void *a, void *b, int *len, MPI_Datatype *dptr){
  double *in = (double*)a;
  double *inout = (double*)b;
  if(*in > 0)
    *inout = *in < *inout ? *in : *inout;
}

void max_func(void *a, void *b, int *len, MPI_Datatype *dptr){
  double *in = (double*)a;
  double *inout = (double*)b;
  if(*in > 0)
    *inout = *in > *inout ? *in : *inout;
}

void sum_func(void *a, void *b, int *len, MPI_Datatype *dptr){
  double *in = (double*)a;
  double *inout = (double*)b;
  if(*in > 0)
    *inout += *in;
}

void tprint(){
  auto begin = std::chrono::steady_clock::now();

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // get timers not present in rank 0
  if (rank == 0){
    for (int r=1; r<size; ++r){
      int n;
      MPI_Recv(&n, 1, MPI_INT, r, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      int *sizes = new int[n];
      MPI_Recv(sizes, n, MPI_INT, r, 43, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      int max_length = *std::max_element(sizes, sizes+n);
      char *name = new char[max_length];
      for (int i=0; i<n; ++i){
        MPI_Recv(name, sizes[i], MPI_CHAR, r, 44, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        entry e;
        e.total = -2000000;
        timers.try_emplace(std::string(name), e);
      }
      delete [] name;
      delete [] sizes;
    }
  }

  // send timers to rank 0 if rank 0 doesn't have it
  if (rank != 0){
    int n = timers.size();
    int *sizes = new int[n];
    int i=0;
    for (auto t: timers) {
      int length = t.first.length()+1;
      sizes[i++] = length;
    }
    MPI_Send(&n, 1, MPI_INT, 0, 42, MPI_COMM_WORLD);
    MPI_Send(sizes, n, MPI_INT, 0, 43, MPI_COMM_WORLD);
    delete [] sizes;
    for (auto t: timers) {
      const char *name = t.first.c_str();
      int length = t.first.length()+1;
      MPI_Send(name, length, MPI_CHAR, 0, 44, MPI_COMM_WORLD);
    }
  }

  // broadcast to every one
  if (rank == 0){
    int n = timers.size();
    int *sizes = new int[n];
    int i=0;
    for (auto t: timers) {
      int length = t.first.length()+1;
      sizes[i++] = length;
    }
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(sizes, n, MPI_INT, 0, MPI_COMM_WORLD);
    delete [] sizes;
    for (auto t: timers) {
      const char *name = t.first.c_str();
      int length = t.first.length()+1;
      MPI_Bcast((char*)name, length, MPI_CHAR, 0, MPI_COMM_WORLD);
    }
  }else{
    int n;
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int *sizes = new int[n];
    MPI_Bcast(sizes, n, MPI_INT, 0, MPI_COMM_WORLD);
    int max_length = *std::max_element(sizes, sizes+n);
    char *name = new char[max_length];
    for (int i=0; i<n; ++i){
      MPI_Bcast(name, sizes[i], MPI_CHAR, 0, MPI_COMM_WORLD);

      entry e;
      e.total = -3000000;
      timers.try_emplace(std::string(name), e);
    }
    delete [] name;
    delete [] sizes;
  }

  MPI_Op min_op, max_op, sum_op;
  MPI_Op_create(min_func, 1, &min_op);
  MPI_Op_create(max_func, 1, &max_op);
  MPI_Op_create(sum_func, 1, &sum_op);

  int length=-1;
  if (rank == 0) {
    // find max timer name length
    for (auto t: timers) {
      int l = t.first.length();
      length = l > length ? l : length;
    }
    printf("| %*s | %10s | %10s | %10s |\n", length, "name", "min(s)", "max(s)", "avg(s)");
  }

  for (auto t: timers) {
    std::string name = t.first;

    double total = sec(t.second.total);
    double min,max,avg;
    MPI_Reduce(&total, &min, 1, MPI_DOUBLE, min_op, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total, &max, 1, MPI_DOUBLE, max_op, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total, &avg, 1, MPI_DOUBLE, sum_op, 0, MPI_COMM_WORLD);
    avg /= (double)size;
    if (rank == 0) {
      printf("| %*s | %10lf | %10lf | %10lf |\n", length, name.c_str(), min, max, avg);
    }
  }

  if (rank == 0) {
    auto now = std::chrono::steady_clock::now();
    auto diff = now - begin;
    double duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
    double duration_sec = sec(duration_ns);
    printf("Time to gather and print: %lfs\n", duration_sec);
  }
}
