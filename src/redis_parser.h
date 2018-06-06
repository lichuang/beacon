#ifndef __REDIS_PARSER_H__
#define __REDIS_PARSER_H__

#include <vector>
#include "redis_item.h"

using namespace std;

class Buffer;
class RedisSession;
class RedisCommand;

// parser statemachine type
enum RedisParserState {
  PARSE_BEGIN,
  PARSE_TYPE,
  PARSE_ARRAY,
  PARSE_STRING,
  PARSE_SIMPLE_STRING,
  PARSE_END,
  PARSE_STATE_NUM
};

class RedisParser {
public:
  RedisParser();
  ~RedisParser();

  bool Parse(RedisSession *session);

private:
  bool parseBegin(RedisCommand *cmd, RedisSession *session);
  bool parseEnd(RedisCommand *cmd, RedisSession *session);
  bool parseType(RedisCommand *cmd, RedisSession *session);
  bool parseSimpleString(RedisCommand *cmd, RedisSession* session);

  RedisItem* newItem(int type);
  RedisItem* nextItem();

  void reset();

private:
  typedef bool (RedisParser::*stateFun)(RedisCommand*, RedisSession*);

  stateFun state_fun_[PARSE_STATE_NUM];

  vector<RedisItem*> items_;
  int item_index_;

  int state_;
  int type_;
  int item_size_;
};

#endif // __REDIS_PARSER_H__
