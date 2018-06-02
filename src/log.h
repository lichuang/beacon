#ifndef __LOG_H__
#define __LOG_H__

static const int kFatalLogLevel = 0;
static const int kErrorLogLevel = 1;
static const int kInfoLogLevel  = 2; 
static const int kDebugLogLevel = 3;

void log(int level, const char* file, int line, const char *format, ...);

#define Fatalf(args...) log(kFatalLogLevel, __FILE__, __LINE__, args)

#endif // __LOG_H__
