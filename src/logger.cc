#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <vector>
#include "logger.h"

static const int kLogBufSize = 500;
static const int kInitFreeList = 100;

static const char* kLogLevelString[] = {
  "F",
  "E",
  "I",
  "D",
  NULL
};

struct LogItem {
  vector<char> buf_;
  int level_;

  LogItem() {
    buf_.reserve(kLogBufSize);
  }
};

Logger::Logger() {
  int i;

  for (i = 0; i < kInitFreeList; ++i) {
    LogItem *item = new LogItem();
    free_.push_back(item);
  }
}

Logger::~Logger() {
}

void Logger::Init(int level, const string& path) {
  level_ = level;
  path_  = path;
}

LogItem* Logger::getFreeItem() {
  if (free_.empty()) {
    int i;

    for (i = 0; i < kInitFreeList; ++i) {
      LogItem *item = new LogItem();
      free_.push_back(item);
    }
  }

  LogItem *item = free_.front();
  free_.pop_front();
  return item;
}

void Logger::Log(int level, const char* file, int line, const char *format, ...) {
  va_list args;
  int n;
  LogItem *item;

  if (level < level_) {
    return;
  }
  va_start(args, format);
  va_end(args);

  item = getFreeItem();
  char *buf = &(item->buf_[0]);
  n  = 0;
  n  = snprintf(buf, kLogBufSize,
                     "%s %s", "eeee", kLogLevelString[level]);
  n += snprintf(buf + n, kLogBufSize - n,
                     " %s:%d ", file, line);
  n += vsnprintf(buf + n, kLogBufSize - n,
                      format, args);
  buf[n] = '\0';

  printf("%s", buf);

  if (level == kFatalLogLevel) {
    abort();
  }
}
