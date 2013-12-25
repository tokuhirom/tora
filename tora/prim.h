#ifndef TORA_PRIM_H_
#define TORA_PRIM_H_

#include <cassert>
#include <stdio.h>

#define PRIM_DECL()      \
 public:                 \
  int refcnt;            \
  void release() {       \
    --refcnt;            \
    if (refcnt == 0) {   \
      delete this;       \
    }                    \
  }                      \
  void retain() {        \
    assert(refcnt >= 0); \
    ++refcnt;            \
  }

#endif  // TORA_PRIM_H_
