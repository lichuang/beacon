#include <pthread.h>
#include <map>
#include <string>
#include "const.h"
#include "string_util.h"
#include "redis_command.h"
#include "redis_item.h"
#include "log.h"

using namespace std;

static map<string, bool> gSupportCommandMap;
static pthread_once_t    gOnce;

static void initCommandMap();
static bool isSupportCommand(const string& cmd);

RedisCommand::RedisCommand()
  : start_(NULL),
    end_(NULL),
    state_(REDIS_COMMAND_RECV),
    item_(NULL),
    need_route_(false) {
  pthread_once(&gOnce, &initCommandMap);
}

RedisCommand::~RedisCommand() {
  FreeBuffers();
}

void RedisCommand::Start(Buffer *buf, int start) {
  if (state_ == REDIS_COMMAND_RECV) {
    start_ = &recv_start_;
  } else {
    start_ = &send_start_;
  }
  start_->buffer_ = buf;
  start_->buffer_->IncrCnt();
  start_->pos_    = start;
}

void RedisCommand::End(Buffer *buf, int end, RedisItem *item) {
  if (state_ == REDIS_COMMAND_RECV) {
    state_ = REDIS_COMMAND_RECV_DONE;
    end_   = &recv_end_;
  } else {
    state_ = REDIS_COMMAND_RECV_RESPONSE_DONE;
    end_   = &send_end_;
  }
  end_->buffer_ = buf;
  end_->pos_    = end;
  item_ = item;
  if (buf != start_->buffer_) {
    end_->buffer_->IncrCnt();
  }
}

void RedisCommand::SetState(int state) {
  switch (state) {
  case REDIS_COMMAND_FORWARD:
    break;
  case REDIS_COMMAND_RECV_RESPONSE:
    // forward client request to redis done,free query buffers
    FreeBuffers();
    break;
  case REDIS_COMMAND_RESPONSE_DONE:
    // response to client done,free response buffers
    FreeBuffers();
    break;
  default:
    // default: do nothing
    break;
  }
  state_ = state;
}

void RedisCommand::MarkResponse(const string& err) {
  Buffer *buffer = GetBuffer(err.length() + 1);
  buffer->Write(err.c_str(), err.length());
  buffer->IncrCnt();
  send_start_.buffer_ = buffer;
  send_start_.pos_ = 0;
  send_end_.buffer_ = buffer;
  send_end_.pos_ = buffer->WritePos();
  start_ = &send_start_;
  end_ = &send_end_;
  state_ = REDIS_COMMAND_RESPONSE;
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
  if (!isSupportCommand(cmd)) {
    Errorf("unspport cmd: %s", cmd.c_str());
    string err = "-(error) ERR unknown command '" + cmd + "'\r\n";
    MarkResponse(err);
    need_route_ = false;
    return true;
  } else {
    need_route_ = true;
  }

  return true;
}

void RedisCommand::ReadyWrite() {
  Buffer *buffer = start_->buffer_;
  buffer->SetReadPos(start_->pos_);

  buffer = buffer->NextBuffer();
  while (buffer != NULL)  {
    buffer->SetReadPos(0);
    if (buffer == end_->buffer_) {
      buffer->SetWritePos(end_->pos_);
    }
  }
}

BufferPos* RedisCommand::NextBufferPos() {
  if (current_.buffer_ == NULL) {
    current_.buffer_ = start_->buffer_;
    current_.pos_    = start_->buffer_->WritePos();
    return &current_;
  }

  current_.buffer_ = current_.buffer_->NextBuffer();
  if (current_.buffer_ == NULL) {
    current_.pos_ = 0;
    return NULL;
  } else if (current_.buffer_ == end_->buffer_){
    current_.pos_ = end_->pos_;
  } else {
    current_.pos_ = current_.buffer_->WritableLength();
  }

  return &current_;
}

void RedisCommand::FreeBuffers() {
  if (start_ == NULL || end_ == NULL) {
    return;
  }
  Buffer *buffer = start_->buffer_;
  Buffer *next;

  buffer->DescCnt();
  while (buffer != NULL && buffer->RefCnt() == 0) {
    next = buffer->NextBuffer();
    FreeBuffer(buffer);
    buffer = next;
  }
  if (end_->buffer_ != start_->buffer_) {
    end_->buffer_->DescCnt();
    if (end_->buffer_->RefCnt() == 0) {
      FreeBuffer(end_->buffer_);
    }
  }

  start_ = end_ = NULL;
}

static void initCommandMap() {
  gSupportCommandMap["SET"] = true;
  gSupportCommandMap["GET"] = true;
}

static bool isSupportCommand(const string& cmd) {
  return gSupportCommandMap.find(cmd) != gSupportCommandMap.end();
}
