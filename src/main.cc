#include "log.h"

int main() {
  Logger::GetInstance()->Init(kDebugLogLevel, "./log/");
  Fatalf("hello");

  return 0;
}
