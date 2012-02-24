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
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] range: %d..%d\n", left->int_value, right->int_value);
    }
    SharedPtr<StrValue> to_s();
    const char *type_str() { return "range"; }

    class iterator : public Value {
    public:
        int counter;
        SharedPtr<RangeValue> parent;
        iterator(SharedPtr<RangeValue> parent_) : Value(VALUE_TYPE_RANGE_ITERATOR) {
            parent = parent_;
            counter = parent->left->int_value;
        }
        void dump(int indent) {
            print_indent(indent);
            printf("[dump] range_iterator(%d):\n", counter);
            parent->dump(indent+1);
        }
        const char *type_str() { return "array_iterator"; }
    };
};

};

#endif // TORA_RANGE_H_
