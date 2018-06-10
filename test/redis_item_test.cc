#include <gtest/gtest.h>
#include "redis_item.h"
#include "redis_session.h"

TEST(RedisItemTests, TestRedisStringItem) {
	Buffer buffer(100); 
	char *start = buffer.Start();
	string ok = "+OK\r\n";
	RedisStringItem item;

	memcpy(start, ok.c_str(), ok.length());
  buffer.AdvanceWrite(ok.length());
	EXPECT_TRUE(item.Parse(&buffer));
  EXPECT_EQ(buffer.ReadableLength(), 1);
  EXPECT_EQ(*buffer.NextRead(), '\n');
}

TEST(RedisItemTests, TestRedisBulkItem) {
	Buffer buffer(100); 
	char *start = buffer.Start();
	string ok = "$6\r\nfoobar\r\n";
	RedisBulkItem item;

	memcpy(start, ok.c_str(), ok.length());
  buffer.AdvanceWrite(ok.length());
  EXPECT_EQ(buffer.ReadableLength(), ok.length());
	EXPECT_TRUE(item.Parse(&buffer));
  EXPECT_EQ(buffer.ReadableLength(), 1);
  EXPECT_EQ(*buffer.NextRead(), '\n');
}

TEST(RedisItemTests, TestRedisArrayItem) {
	Buffer buffer(100); 
	char *start = buffer.Start();
	string ok = "*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n";
	RedisArrayItem item;

	memcpy(start, ok.c_str(), ok.length());
  buffer.AdvanceWrite(ok.length());
  EXPECT_EQ(buffer.ReadableLength(), ok.length());
	EXPECT_TRUE(item.Parse(&buffer));
  EXPECT_EQ(buffer.ReadableLength(), 1);
  EXPECT_EQ(*buffer.NextRead(), '\n');
}
