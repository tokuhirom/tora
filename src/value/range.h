#ifndef TORA_RANGE_H_
#define TORA_RANGE_H_

#include "../value.h"
#include "../shared_ptr.h"

namespace tora {

class RangeValue: public Value {
public:
    SharedPtr<IntValue> left;
    SharedPtr<IntValue> right;

    RangeValue(SharedPtr<IntValue> l, SharedPtr<IntValue> r) : Value(VALUE_TYPE_RANGE) {
        left = l;
        right = r;
    }
    const char *type_str() { return "range"; }

    class iterator : public Value {
    public:
        int counter;
        SharedPtr<RangeValue> parent;
        iterator(SharedPtr<RangeValue> parent_) : Value(VALUE_TYPE_RANGE_ITERATOR) {
            parent = parent_;
            counter = parent->left->int_value;
        }
        const char *type_str() { return "array_iterator"; }
    };
};

};

#endif // TORA_RANGE_H_
