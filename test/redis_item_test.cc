#include <gtest/gtest.h>
#include "redis_item.h"
#include "redis_session.h"

TEST(RedisItemTests, TestRedisStringItem) {
	RedisSession *session = new RedisSession(0, "0.0.0.0", 123, NULL);
	Buffer* buffer = session->QueryBuffer();
	char *start = buffer->Start();
	string ok = "+OK\r\n";
	string wrong = "OK\r\n";
	RedisCommand cmd;
	RedisStringItem item(&cmd, session);

	memcpy(start, ok.c_str(), ok.length());
  buffer->AdvanceWrite(ok.length());
	EXPECT_TRUE(session->hasUnprocessedQueryData());
  EXPECT_EQ(buffer->ReadableLength(), ok.length());
	EXPECT_TRUE(item.Parse());
  EXPECT_EQ(buffer->ReadableLength(), 0);
}

TEST(RedisItemTests, TestRedisBulkItem) {
	RedisSession *session = new RedisSession(0, "0.0.0.0", 123, NULL);
	Buffer* buffer = session->QueryBuffer();
	char *start = buffer->Start();
	string ok = "$6\r\nfoobar\r\n";
	RedisCommand cmd;
	RedisBulkItem item(&cmd, session);

	memcpy(start, ok.c_str(), ok.length());
  buffer->AdvanceWrite(ok.length());
	EXPECT_TRUE(session->hasUnprocessedQueryData());
  EXPECT_EQ(buffer->ReadableLength(), ok.length());
	EXPECT_TRUE(item.Parse());
  EXPECT_EQ(buffer->ReadableLength(), 0);
}

TEST(RedisItemTests, TestRedisArrayItem) {
	RedisSession *session = new RedisSession(0, "0.0.0.0", 123, NULL);
	Buffer* buffer = session->QueryBuffer();
	char *start = buffer->Start();
	string ok = "*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n";
	RedisCommand cmd;
	RedisArrayItem item(&cmd, session);

	memcpy(start, ok.c_str(), ok.length());
  buffer->AdvanceWrite(ok.length());
	EXPECT_TRUE(session->hasUnprocessedQueryData());
  EXPECT_EQ(buffer->ReadableLength(), ok.length());
	EXPECT_TRUE(item.Parse());
  EXPECT_EQ(buffer->ReadableLength(), 0);
}
