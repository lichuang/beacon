#include <gtest/gtest.h>
#include "redis_item.h"
#include "redis_parser.h"
#include "redis_session.h"

TEST(RedisParserTests, TestParse) {
	RedisSession *session = new RedisSession(0, "0.0.0.0", 123, NULL);
	Buffer* buffer = session->QueryBuffer();
	char *start = buffer->Start();
	string ok = "+OK\r\n";
  RedisParser parser(session);

	memcpy(start, ok.c_str(), ok.length());
  buffer->AdvanceWrite(ok.length());
  EXPECT_TRUE(parser.Parse());

  list<RedisCommand*>* cmds = session->getWaintingCommands();

  EXPECT_EQ(cmds->size(), 1);
}
