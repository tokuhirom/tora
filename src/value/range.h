#ifndef TORA_RANGE_H_
#define TORA_RANGE_H_

#include "../value.h"
#include "../shared_ptr.h"

namespace tora {

class RangeValue: public Value {
public:
    SharedPtr<IntValue> left;
    SharedPtr<IntValue> right;

    RangeValue(SharedPtr<IntValue> l, SharedPtr<IntValue> r) {
        left = l;
        right = r;
    }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] range: %d..%d\n", left->int_value, right->int_value);
    }
    SharedPtr<StrValue> to_s();
    const char *type_str() { return "range"; }
};

};

#endif // TORA_RANGE_H_
