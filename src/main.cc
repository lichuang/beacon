#include "config.h"
#include "engine.h"
#include "log.h"
#include "redis_session.h"
#include "server.h"

int main() {
  Config config;
  config.factory_ = new RedisSessionFactory();
  Logger::GetInstance()->Init(kDebugLogLevel, "./log/");
  Engine engine(config.setsize_);
  Server server(&config, &engine);
  return server.Run();
}
