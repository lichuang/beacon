#include <stdio.h>
#include <stdarg.h>
#include "logger.h"

/*
static const char* kLogLevelString[] = {
  "F",
  "E",
  "I",
  "D",
  NULL
};
*/

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
  char buff[1024];
  int n;

  va_start(args, format);
  va_end(args);

  n  = 0;
  n  = snprintf(buff, 1024,
                     "%s", "eeee");
  n += snprintf(buff + n, 1024 - n,
                     " %s:%d ", file, line);
  n += vsnprintf(buff + n, 1024 - n,
                      format, args);

  printf("%s", buff);
}
