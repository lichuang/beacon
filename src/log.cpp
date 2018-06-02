#include "log.h"

static const char* kLogLevelString[] = {
  "Fatal",
  "Error",
  "Info",
  "Debug",
  NULL
}

void log(int level, const char* file, int line, const char *format, ...) {
}
