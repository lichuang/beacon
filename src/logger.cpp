#include <stdio.h>
#include "logger.h"

static const char* kLogLevelString[] = {
  "Fatal",
  "Error",
  "Info",
  "Debug",
  NULL
}

Logger::Logger() {
}

Logger::~Logger() {
}

void Logger::Init(int level, const string& path) {
  level_ = level;
  path_  = path;
}

void Logger::Log(int level, const char* file, int line, const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_end(args);

  char buf[1024];
  n  = 0;
  n  = snprintf(buff, 1024,
                     "%s", logger->time_buff);
  n += snprintf(buff + n, 1024 - n,
                     " %s:%d ", file, line);
  n += vsnprintf(buff + n, 1024 - n,
                      format, args);

  printf("%s", buf);
}
