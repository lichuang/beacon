#ifndef __REDIS_PARSER_H__
#define __REDIS_PARSER_H__

#include <vector>
#include "redis_item.h"
#include "redis_parser.h"

using namespace std;

class Buffer;
class RedisInfo;
class RedisCommand;

// parser statemachine type
enum RedisParserState {
  PARSE_BEGIN,
  PARSE_ITEM,
  PARSE_END,
  PARSE_STATE_NUM
};

class RedisParser {
public:
  RedisParser(RedisInfo *info);
  ~RedisParser();

  bool Parse();

private:
  bool parseBegin();
  bool parseItem();
  bool parseEnd();

  void reset();

private:
  typedef bool (RedisParser::*stateFun)();

  stateFun state_fun_[PARSE_STATE_NUM];

  RedisItem* item_;
  RedisInfo *info_;
  RedisCommand *cmd_;

  int state_;
  int type_;
  bool parse_cmd_done_;
};

bool ParseType(char c, int *type);

#endif // __REDIS_PARSER_H__
