#ifndef __CONST_H__
#define __CONST_H__

// net error buffer len
const int kNetErrorLen = 256;

// max accept per I/O callback
const int kMaxAcceptPerCall = 1000;

// client query buffer size
const int kQueryBufferLen = 16 * 1024;

// client address buffer length
const int kAddressLen = 20;

// redis data type prefix
enum {
  kRedisStringPrefix = '+',
  kRedisErrorPrefix  = '-',
  kRedisIntPrefix    = ':',
  kRedisBulkPrefix   = '$',
  kRedisArrayPrefix  = '*',
};

// tcp connect status
enum {
  kConnected,
  kConnecting,
  kDisconnected,
};

// redis cmd request/response mode
enum {
  REDIS_REQ_MODE,
  REDIS_REP_MODE,
  REDIS_NONE_MODE,
};

#endif // __CONST_H__
