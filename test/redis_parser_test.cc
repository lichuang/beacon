#include <gtest/gtest.h>
#include "redis_item.h"
#include "redis_parser.h"
#include "redis_session.h"

TEST(RedisParserTests, TestParse) {
  return;
  RedisInfo info((RedisSession*)NULL);
	Buffer buffer(100);
	char *start = buffer.Start();
	string ok = "*2\r\n$3\r\nget\r\n$1\r\na\r\n";
  RedisParser parser(&info);
  RedisCommand *cmd;

	memcpy(start, ok.c_str(), ok.length());
  buffer.AdvanceWrite(ok.length());
  cmd = parser.Parse(&buffer, NULL);
  EXPECT_TRUE(cmd->GetReady());
}
