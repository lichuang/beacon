#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <vector>

using namespace std;

class Bitmap {
public:
  Bitmap(int size);
  ~Bitmap();

  int Next();
  void Clear(int x);

private:
  vector<int> bits_;
  int next_;
};

#endif // __BITMAP_H__
