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

#endif // __CONST_H__
