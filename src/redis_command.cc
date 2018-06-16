#include <pthread.h>
#include <map>
#include <string>
#include "string_util.h"
#include "redis_command.h"
#include "redis_item.h"
#include "log.h"

using namespace std;

static map<string, bool> gCommandMap;
static pthread_once_t    gOnce;

static void initCommandMap();

RedisCommand::RedisCommand()
  : status_(REDIS_COMMAND_NONE),
    item_(NULL),
    need_route_(false) {
  pthread_once(&gOnce, &initCommandMap);
}

RedisCommand::~RedisCommand() {
}

void RedisCommand::Init(Buffer *buf, int start) {
  start_.buffer_ = buf;
  start_.pos_    = start;
}

void RedisCommand::End(Buffer *buf, int end, RedisItem *item) {
  end_.buffer_ = buf;
  end_.pos_    = end;
  status_ = REDIS_COMMAND_READY;
  item_ = item;
}

bool RedisCommand::Parse() {
  if (item_ == NULL) {
    return false;
  }
  if (item_->GetType() != REDIS_ARRAY) {
    Errorf("redis first item MUST be array type");
    return false;
  }
  string cmd;
  RedisArrayItem *aitem = (RedisArrayItem*)item_;
  aitem->array_[0]->GetValue(&cmd);
  Infof("cmd: %s", cmd.c_str());
  ToUpper(&cmd);
  if (gCommandMap.find(cmd) == gCommandMap.end()) {
    Errorf("unspport cmd: %s", cmd.c_str());
    string err = "-(error) ERR unknown command '" + cmd + "'\r\n";
    Buffer *buffer = start_.buffer_;
    buffer->Reset();
    buffer->Write(err.c_str(), err.length());
    need_route_ = false;
    return true;
  }

  return true;
}

void RedisCommand::ReadyWrite() {
  Buffer *buffer = start_.buffer_;
  buffer->SetReadPos(start_.pos_);

  buffer = buffer->NextBuffer();
  while (buffer != NULL)  {
    buffer->SetReadPos(0);
    if (buffer == end_.buffer_) {
      buffer->SetWritePos(end_.pos_);
    }
  }
}

BufferPos* RedisCommand::NextBufferPos() {
  if (current_.buffer_ == NULL) {
    current_.buffer_ = start_.buffer_;
    current_.pos_    = start_.buffer_->WritePos();
    return &current_;
  }

  current_.buffer_ = current_.buffer_->NextBuffer();
  if (current_.buffer_ == NULL) {
    current_.pos_ = 0;
    return NULL;
  } else if (current_.buffer_ == end_.buffer_){
    current_.pos_ = end_.pos_;
  } else {
    current_.pos_ = current_.buffer_->WritableLength();
  }

  return &current_;
}

void RedisCommand::FreeBuffers() {
  Buffer *buffer = start_.buffer_;
  while (buffer != NULL) {
    buffer->DescCnt();
    if (buffer == end_.buffer_) {
      break;
    }
    buffer = buffer->NextBuffer();
  }
}

void initCommandMap() {
  gCommandMap["SET"] = true;
}
