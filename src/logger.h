#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "singleton.h"

class Logger : public Singleton<Logger> {
public:
  Logger();
  virtual ~Logger();

private:
};

#endif // __LOGGER_H__
