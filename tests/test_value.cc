#include "nanotap.h"
#include "../tora/value.h"

using namespace tora;

static inline void X(const char *src, int expected) {
  std::string src_s(src);
  MortalStrValue v( src_s );
  int iev = v->to_int();
  is(iev, expected);
}

int main() {
    X("1", 1);
    X("19", 19);
    X("4649", 4649);
    done_testing();
}
