#ifndef __ERR_CODE_H__
#define __ERR_CODE_H__

typedef int errno_t;

const int kAgain        = 2;
const int kInProgress   = 1;
const int kOk           = 0;
const int kError        = -1;
const int kFdOutOfRange = -2;
const int kNoTimeId     = -3;
const int kEOF          = -4;

#endif  // __ERR_CODE_H__
