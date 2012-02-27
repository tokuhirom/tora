#include "nanotap.h"
#include "../src/value.h"

using namespace tora;

inline void X(const char *src, int expected) {
    SharedPtr<StrValue> v = new StrValue(src);
    SharedPtr<Value> iev = v->to_int();
    ok(!iev->is_exception());
    is(iev->upcast<IntValue>()->int_value, expected);
}

int main() {
    X("1", 1);
    X("19", 19);
    X("4649", 4649);
    done_testing();
}
