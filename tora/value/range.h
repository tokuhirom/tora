#ifndef TORA_RANGE_H_
#define TORA_RANGE_H_

#include "../value.h"
#include "../shared_ptr.h"
#include "int.h"

namespace tora {

class RangeValue: public Value {
public:
    SharedPtr<IntValue> left;
    SharedPtr<IntValue> right;

    RangeValue(SharedPtr<IntValue> l, SharedPtr<IntValue> r);

    class Iterator : public Value {
    public:
        int counter;
        SharedPtr<RangeValue> parent;
        Iterator(SharedPtr<RangeValue> parent_) : Value(VALUE_TYPE_RANGE_ITERATOR) {
            parent  = parent_;
            counter = parent->left->int_value();
        }
    };
};

};

#endif // TORA_RANGE_H_
