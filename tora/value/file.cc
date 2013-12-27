#include <stdio.h>
#include "file.h"

using namespace tora;

static FILE* SELF(Value* v) {
  return static_cast<FILE*>(v->ptr_value_);
}

void tora::file_close(Value* self) {
  FILE* fp = SELF(self);
  if (fp) {
    fclose(fp);
    self->ptr_value_ = NULL;
  }
}

std::string tora::file_slurp(Value* self)
{
  // slurp
  std::string s;
  const int bufsiz = 4096;
  char buffer[bufsiz];
  size_t n;
  FILE* fp = SELF(self);
  while ((n = fread(buffer, sizeof(char), bufsiz, fp)) != 0) {
    s.append(buffer, n);
  }
  if (feof(fp)) {
    return s;
  } else {
    // err?
    throw new ErrnoExceptionValue(tora::get_errno());
  }
}
