#ifndef SIMPLE_TIMER_H
#define SIMPLE_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif
  void tstart(const char* name);
  void tstop(const char* name);
  void tprint();
#ifdef __cplusplus
}
#endif

#endif
