#include "bitmap.h"

Bitmap::Bitmap(int size)
  : next_(1) {
  bits_.resize(size / 32 + 1);    
}

Bitmap::~Bitmap() {
}

int Bitmap::Next() {
  size_t cnt = 0;
  while (cnt < bits_.size()) {
    int index = next_ / 32;
    int temp  = next_ % 32;
    if (bits_[index] & (1 << temp)) {
      return next_;
    }
    ++next_;
    ++cnt;
  }
  return -1;
}

void Bitmap::Clear(int x) {
  bits_[x / 32] &= ~(1 << (x % 32));
}
