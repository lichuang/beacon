#include <gtest/gtest.h>
#include "redis_item.h"
#include "redis_session.h"

TEST(RedisItemTests, TestRedisStringItem) {
  // normal case
  {
    Buffer buffer(100); 
    string ok = "+OK\r\n";
    RedisStringItem item;

    buffer.Write(ok.c_str(), ok.length());
    EXPECT_TRUE(item.Parse(&buffer));
    EXPECT_TRUE(item.Ready());
    EXPECT_EQ(buffer.ReadableLength(), 0);

    // check item pos
    EXPECT_EQ(item.GetItemStartBuffer(), &buffer);
    EXPECT_EQ(item.GetItemStartPos(), 0);
    EXPECT_EQ(item.GetItemEndBuffer(), &buffer);
    EXPECT_EQ(item.GetItemEndPos(), buffer.ReadPos() - 1);
    EXPECT_EQ(buffer.Start()[item.GetItemStartPos()], '+');
    EXPECT_EQ(buffer.Start()[item.GetItemEndPos()], '\n');

    // check item value pos
    EXPECT_EQ(item.GetValueStartBuffer(), &buffer);
    EXPECT_EQ(item.GetValueStartPos(), 1);
    EXPECT_EQ(item.GetValueEndBuffer(), &buffer);
    EXPECT_EQ(item.GetValueEndPos(), buffer.ReadPos() - 2);
    EXPECT_EQ(buffer.Start()[item.GetValueStartPos()], 'O');
    EXPECT_EQ(buffer.Start()[item.GetValueEndPos()], '\r');

    // check value
    string value;
    item.GetValue(&value);
    EXPECT_EQ(value, "OK");
  }
  // multi buffer case
  {
    Buffer buf1(100), buf2(100), buf3(100); 
    buf1.SetNextBuffer(&buf2);
    buf2.SetNextBuffer(&buf3);
    
    string ok = "+Hello world\r\n";
    int start_pos = 0, mid_pos = ok.length() / 3, end_pos = mid_pos + 2;
    int buf1_len = mid_pos - start_pos;
    int buf2_len = end_pos - mid_pos;
    int buf3_len = ok.length() - end_pos;

    RedisStringItem item;

    buf1.Write(ok.c_str() + start_pos, buf1_len);
    buf2.Write(ok.c_str() + mid_pos, buf2_len);
    buf3.Write(ok.c_str() + end_pos, buf3_len);

    EXPECT_TRUE(item.Parse(&buf1));
    EXPECT_FALSE(item.Ready());
    EXPECT_EQ(buf1.ReadableLength(), 0);

    EXPECT_TRUE(item.Parse(&buf2));
    EXPECT_FALSE(item.Ready());
    EXPECT_EQ(buf2.ReadableLength(), 0);

    EXPECT_TRUE(item.Parse(&buf3));
    EXPECT_TRUE(item.Ready());
    EXPECT_EQ(buf3.ReadableLength(), 0);

    // check value
    string value;
    item.GetValue(&value);
    EXPECT_EQ(value, "Hello world");

    // check item pos
    EXPECT_EQ(item.GetItemStartBuffer(), &buf1);
    EXPECT_EQ(item.GetItemStartPos(), 0);
    EXPECT_EQ(item.GetItemEndBuffer(), &buf3);
    EXPECT_EQ(item.GetItemEndPos(), buf3.ReadPos() - 1);
    EXPECT_EQ(buf1.Start()[item.GetItemStartPos()], '+');
    EXPECT_EQ(buf3.Start()[item.GetItemEndPos()], '\n');

    // check item value pos
    EXPECT_EQ(item.GetValueEndBuffer(), &buf3);
    EXPECT_EQ(item.GetValueEndPos(), buf3.ReadPos() - 2);
    EXPECT_EQ(item.GetValueStartBuffer()->Start()[item.GetValueStartPos()], 'H');
    EXPECT_EQ(buf3.Start()[item.GetValueEndPos()], '\r');
  }
}

TEST(RedisItemTests, TestRedisBulkItem) {
  string ok = "$6\r\nfoobar\r\n";
  // normal case
  {
    Buffer buffer(100); 
    RedisBulkItem item;

    buffer.Write(ok.c_str(), ok.length());
    EXPECT_TRUE(item.Parse(&buffer));
    EXPECT_TRUE(item.Ready());
    EXPECT_EQ(buffer.ReadableLength(), 0);

    // check value
    string value;
    item.GetValue(&value);
    EXPECT_EQ(value, "foobar");

    // check item pos
    EXPECT_EQ(item.GetItemStartBuffer(), &buffer);
    EXPECT_EQ(item.GetItemStartPos(), 0);
    EXPECT_EQ(item.GetItemEndBuffer(), &buffer);
    EXPECT_EQ(item.GetItemEndPos(), buffer.ReadPos());
    EXPECT_EQ(buffer.Start()[item.GetItemStartPos()], '$');
    EXPECT_EQ(buffer.Start()[item.GetItemEndPos() - 1], '\n');

    // check item value pos
    EXPECT_EQ(item.GetValueStartBuffer(), &buffer);
    EXPECT_EQ(item.GetValueStartPos(), 4);
    EXPECT_EQ(item.GetValueEndBuffer(), &buffer);
    EXPECT_EQ(item.GetValueEndPos(), buffer.ReadPos() - 2);
    EXPECT_EQ(buffer.Start()[item.GetValueStartPos()], 'f');
    EXPECT_EQ(buffer.Start()[item.GetValueEndPos()], '\r');
  }
  // multi buffer case
  {
    Buffer buf1(100), buf2(100), buf3(100); 
    buf1.SetNextBuffer(&buf2);
    buf2.SetNextBuffer(&buf3);
    
    int start_pos = 0, mid_pos = ok.length() / 3, end_pos = mid_pos + 2;
    int buf1_len = mid_pos - start_pos;
    int buf2_len = end_pos - mid_pos;
    int buf3_len = ok.length() - end_pos;

    RedisBulkItem item;

    buf1.Write(ok.c_str() + start_pos, buf1_len);
    buf2.Write(ok.c_str() + mid_pos, buf2_len);
    buf3.Write(ok.c_str() + end_pos, buf3_len);

    EXPECT_TRUE(item.Parse(&buf1));
    EXPECT_FALSE(item.Ready());
    EXPECT_EQ(buf1.ReadableLength(), 0);

    EXPECT_TRUE(item.Parse(&buf2));
    EXPECT_FALSE(item.Ready());
    EXPECT_EQ(buf2.ReadableLength(), 0);

    EXPECT_TRUE(item.Parse(&buf3));
    EXPECT_TRUE(item.Ready());
    EXPECT_EQ(buf3.ReadableLength(), 0);

    // check value
    string value;
    item.GetValue(&value);
    EXPECT_EQ(value, "foobar");

    // check item pos
    EXPECT_EQ(item.GetItemStartBuffer(), &buf1);
    EXPECT_EQ(item.GetItemStartPos(), 0);
    EXPECT_EQ(item.GetItemEndBuffer(), &buf3);
    EXPECT_EQ(item.GetItemEndPos(), buf3.ReadPos());
    EXPECT_EQ(buf1.Start()[item.GetItemStartPos()], '$');
    EXPECT_EQ(buf3.Start()[item.GetItemEndPos() - 1], '\n');

    // check item value pos
    EXPECT_EQ(item.GetValueEndBuffer(), &buf3);
    EXPECT_EQ(item.GetValueEndPos(), buf3.ReadPos() - 2);
    EXPECT_EQ(item.GetValueStartBuffer()->Start()[item.GetValueStartPos()], 'f');
    EXPECT_EQ(buf3.Start()[item.GetValueEndPos()], '\r');
  }
}

TEST(RedisItemTests, TestRedisIntItem) {
  string ok = ":1000\r\n";
  // normal case
  {
    Buffer buffer(100); 
    RedisIntItem item;

    buffer.Write(ok.c_str(), ok.length());
    EXPECT_TRUE(item.Parse(&buffer));
    EXPECT_TRUE(item.Ready());
    EXPECT_EQ(buffer.ReadableLength(), 0);

    // check item pos
    EXPECT_EQ(item.GetItemStartBuffer(), &buffer);
    EXPECT_EQ(item.GetItemStartPos(), 0);
    EXPECT_EQ(item.GetItemEndBuffer(), &buffer);
    EXPECT_EQ(item.GetItemEndPos(), buffer.ReadPos() - 1);
    EXPECT_EQ(buffer.Start()[item.GetItemStartPos()], ':');
    EXPECT_EQ(buffer.Start()[item.GetItemEndPos()], '\n');

    // check item value pos
    EXPECT_EQ(item.GetValueStartBuffer(), &buffer);
    EXPECT_EQ(item.GetValueStartPos(), 1);
    EXPECT_EQ(item.GetValueEndBuffer(), &buffer);
    EXPECT_EQ(item.GetValueEndPos(), buffer.ReadPos() - 2);
    EXPECT_EQ(buffer.Start()[item.GetValueStartPos()], '1');
    EXPECT_EQ(buffer.Start()[item.GetValueEndPos()], '\r');

    // check value
    string value;
    item.GetValue(&value);
    EXPECT_EQ(value, "1000");
  }
  {
    Buffer buf1(100), buf2(100), buf3(100); 
    buf1.SetNextBuffer(&buf2);
    buf2.SetNextBuffer(&buf3);
    
    int start_pos = 0, mid_pos = ok.length() / 3, end_pos = mid_pos + 2;
    int buf1_len = mid_pos - start_pos;
    int buf2_len = end_pos - mid_pos;
    int buf3_len = ok.length() - end_pos;

    RedisIntItem item;

    buf1.Write(ok.c_str() + start_pos, buf1_len);
    buf2.Write(ok.c_str() + mid_pos, buf2_len);
    buf3.Write(ok.c_str() + end_pos, buf3_len);

    EXPECT_TRUE(item.Parse(&buf1));
    EXPECT_FALSE(item.Ready());
    EXPECT_EQ(buf1.ReadableLength(), 0);

    EXPECT_TRUE(item.Parse(&buf2));
    EXPECT_FALSE(item.Ready());
    EXPECT_EQ(buf2.ReadableLength(), 0);

    EXPECT_TRUE(item.Parse(&buf3));
    EXPECT_TRUE(item.Ready());
    EXPECT_EQ(buf3.ReadableLength(), 0);

    // check value
    string value;
    item.GetValue(&value);
    EXPECT_EQ(value, "1000");

    // check item pos
    EXPECT_EQ(item.GetItemStartBuffer(), &buf1);
    EXPECT_EQ(item.GetItemStartPos(), 0);
    EXPECT_EQ(item.GetItemEndBuffer(), &buf3);
    EXPECT_EQ(item.GetItemEndPos(), buf3.ReadPos() - 1);
    EXPECT_EQ(buf1.Start()[item.GetItemStartPos()], ':');
    EXPECT_EQ(buf3.Start()[item.GetItemEndPos()], '\n');

    // check item value pos
    EXPECT_EQ(item.GetValueEndBuffer(), &buf3);
    EXPECT_EQ(item.GetValueEndPos(), buf3.ReadPos() - 2);
    EXPECT_EQ(item.GetValueStartBuffer()->Start()[item.GetValueStartPos()], '1');
    EXPECT_EQ(buf3.Start()[item.GetValueEndPos()], '\r');
  }
}

TEST(RedisItemTests, TestRedisArrayItem) {
	string ok = "*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n";
  // normal case
  {
    Buffer buffer(100); 
    RedisArrayItem item;

    buffer.Write(ok.c_str(), ok.length());
    EXPECT_TRUE(item.Parse(&buffer));
    EXPECT_TRUE(item.Ready());
    EXPECT_EQ(buffer.ReadableLength(), 0);
    EXPECT_EQ(item.array_.size(), 2);

    // check item pos
    EXPECT_EQ(item.GetItemStartBuffer(), &buffer);
    EXPECT_EQ(item.GetItemStartPos(), 0);
    EXPECT_EQ(item.GetItemEndBuffer(), &buffer);
    EXPECT_EQ(item.GetItemEndPos(), buffer.ReadPos());
    EXPECT_EQ(buffer.Start()[item.GetItemStartPos()], '*');
    EXPECT_EQ(buffer.Start()[item.GetItemEndPos() - 1], '\n');

    /*
    // check item value pos
    EXPECT_EQ(item.GetValueStartBuffer(), &buffer);
    EXPECT_EQ(item.GetValueStartPos(), 1);
    EXPECT_EQ(item.GetValueEndBuffer(), &buffer);
    EXPECT_EQ(item.GetValueEndPos(), buffer.ReadPos() - 2);
    EXPECT_EQ(buffer.Start()[item.GetValueStartPos()], '1');
    EXPECT_EQ(buffer.Start()[item.GetValueEndPos()], '\r');

    // check value
    string value;
    item.GetValue(&value);
    EXPECT_EQ(value, "1000");
    */
  }
}
