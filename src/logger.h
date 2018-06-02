#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <string>
#include <list>
#include "singleton.h"

using namespace std;

struct LogItem;

class Logger : public Singleton<Logger> {
public:
  Logger();
  virtual ~Logger();

  void Init(int level, const string& path);
  void Log(int level, const char* file, int line, const char *format, ...);

private:
  LogItem* getFreeItem();

private:
  int level_;
  string path_;

  list<LogItem*> items_[2];
  list<LogItem*> *current_;
  list<LogItem*> *wait_;
  list<LogItem*> free_;
};

#endif // __LOGGER_H__
