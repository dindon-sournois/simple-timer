#include <mpi.h>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <map>
#include <algorithm>

#ifdef CUDA
#include <nvtx3/nvToolsExt.h>
#endif
#ifdef ROCM
#include <roctracer/roctx.h>
#endif

#include "simple_timer.h"

// each rank stores their own timers
typedef struct {
  std::chrono::steady_clock::time_point begin;
  int color;
  double total;
  bool first;
} timer_entry;

std::map<std::string, timer_entry> timers;

// rank 0 gather results and compute min/max/avg for all ranks
typedef struct {
  double min;
  double max;
  double sum;
  int num;
} rank_entry;

#ifdef CUDA
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

bool skip_first = false;

void tinit(){
  if (const char* skipe = std::getenv("SIMPLE_TIMER_SKIP_FIRST")){
    std::string skip(skipe);
    if (skip == "1" || skip == "true" || skip == "yes" || skip == "y"){
      skip_first = true;
    }
  }
}

void tstart(const char* name){
  static bool first = true;
  if (first)
    tinit();
  first = false;

  std::string s = std::string(name);
  auto now = std::chrono::steady_clock::now();
  // init total to 0 at first insertion
  auto ret = timers.try_emplace(s,timer_entry{now,0,0.0,true});
  bool new_entry = ret.second;
  if(new_entry) {
#ifdef CUDA
    timers[s].color = (color_counter++)%num_colors;
#endif
#ifdef ROCM
    // TODO: roctx colors?
#endif
  } else {
    timers[s].begin = now;
  }
#ifdef CUDA
  int color_id = timers[s].color;
  nvtxEventAttributes_t eventAttrib = {0};
  eventAttrib.version = NVTX_VERSION;
  eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
  eventAttrib.colorType = NVTX_COLOR_ARGB;
  eventAttrib.color = colors[color_id];
  eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
  eventAttrib.message.ascii = name;
  nvtxRangePushEx(&eventAttrib);
#endif
#ifdef ROCM
  // TODO: any roctx colors?
  roctxRangePush(name);
#endif
}

void tstop(const char* name){
  auto now = std::chrono::steady_clock::now();
  auto diff = now - timers[name].begin;
  double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();

  if (timers[name].first && skip_first){
    timers[name].first = false;
  }else{
    timers[name].total += duration;
  }

#ifdef CUDA
  nvtxRangePop();
#endif
#ifdef ROCM
  roctxRangePop();
#endif
}

inline double sec(double nano){
  return nano / 1000000000.0;
}

// this needs to be called by all ranks
void tprint(){
  MPI_Barrier(MPI_COMM_WORLD);
  auto begin = std::chrono::steady_clock::now();

  // rank 0 gather results and compute min/max/avg for all ranks
  std::map<std::string, rank_entry> stats;

  int rank, nranks;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nranks);

  // fetch timers to rank 0, handling cases of rank-divergent path
  if (rank == 0){
    // init min/max/avg stats with rank 0 own timer values
    for (auto t: timers) {
      double total = t.second.total;
      stats.emplace(t.first, rank_entry{total,total,total,1});
    }
    for (int r=1; r<nranks; ++r){
      int ntimers;
      MPI_Recv(&ntimers, 1, MPI_INT, r, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      int *timer_name_lengths = new int[ntimers];
      MPI_Recv(timer_name_lengths, ntimers, MPI_INT, r, 43, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      int max_length = *std::max_element(timer_name_lengths, timer_name_lengths+ntimers);
      char *name = new char[max_length];
      for (int i=0; i<ntimers; ++i){
        double total;
        MPI_Recv(name, timer_name_lengths[i], MPI_CHAR, r, 44, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&total, 1, MPI_DOUBLE, r, 45, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::string s = std::string(name);
        if(!stats.try_emplace(s, rank_entry{total,total,total,1}).second) { // already present, we recompute stats
          stats[s].min = stats[s].min < total ? stats[s].min : total;
          stats[s].max = stats[s].max > total ? stats[s].max : total;
          stats[s].sum += total;
          ++stats[s].num;
        } // else: a new entry is added with this rank's stats as a starting point
      }
      delete [] name;
      delete [] timer_name_lengths;
    }
  }

  // send timers to rank 0
  if (rank != 0){
    int ntimers = timers.size();
    int *timer_name_lengths = new int[ntimers];
    int i=0;
    for (auto t: timers) {
      int length = t.first.length()+1;
      timer_name_lengths[i++] = length;
    }
    MPI_Send(&ntimers, 1, MPI_INT, 0, 42, MPI_COMM_WORLD);
    MPI_Send(timer_name_lengths, ntimers, MPI_INT, 0, 43, MPI_COMM_WORLD);
    for (auto t: timers) {
      const char *name = t.first.c_str();
      int length = t.first.length()+1;
      MPI_Send(name, length, MPI_CHAR, 0, 44, MPI_COMM_WORLD);
      MPI_Send(&t.second.total, 1, MPI_DOUBLE, 0, 45, MPI_COMM_WORLD);
    }
    delete [] timer_name_lengths;
  }

  int length=-1;
  if (rank == 0) {
    // find max timer name length
    for (auto t: stats) {
      int l = t.first.length();
      length = l > length ? l : length;
    }
    printf("| %*s | %10s | %10s | %10s |\n", length, "name", "min(s)", "max(s)", "avg(s)");

    for (auto t: stats) {
      std::string name = t.first;

      double min = sec(t.second.min);
      double max = sec(t.second.max);
      double avg = sec(t.second.sum/(double)t.second.num);
      printf("| %*s | %10lf | %10lf | %10lf |\n", length, name.c_str(), min, max, avg);
    }

    auto now = std::chrono::steady_clock::now();
    auto diff = now - begin;
    double duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
    double duration_sec = sec(duration_ns);
    printf("Time to gather and print: %lfs\n", duration_sec);
  }
}
