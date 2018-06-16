#include <ctype.h>
#include <string.h>
#include "string_util.h"

void ToUpper(string *str) {
  if (str == NULL) {
    return;
  }

  int len = str->length();
  int i;
  for (i = 0; i < len; ++i) {
    if (!islower((*str)[i])) {
      continue;
    }
    (*str)[i] = toupper((*str)[i]);
  }
}
