#include <sys/time.h>
#include "errcode.h"
#include "util.h"

uint64_t GetCurrentMs() {
  struct timeval tv;
  time_t sec;
  msec_t msec;

  if (gettimeofday(&tv) < 0) {
    return kError;
  }

  sec = tv.tv_sec;
  msec = tv.tv_usec / 1000;

  return (msec_t) sec * 1000 + msec;
}
