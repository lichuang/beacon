#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <string>
#include "singleton.h"

using namespace std;

class Logger : public Singleton<Logger> {
public:
  Logger();
  virtual ~Logger();

  void Init(int level, const string& path);
  void Log(int level, const char* file, int line, const char *format, ...);

private:
  int level_;
  string path_;
};

#endif // __LOGGER_H__
