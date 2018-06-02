#ifndef __LOG_H__
#define __LOG_H__

#include "logger.h"

const int kFatalLogLevel = 0;
const int kErrorLogLevel = 1;
const int kInfoLogLevel  = 2; 
const int kDebugLogLevel = 3;

#define Fatalf(args...) Logger::GetInstance()->Log(kFatalLogLevel, __FILE__, __LINE__, args)

#endif // __LOG_H__
