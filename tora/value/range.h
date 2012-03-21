#ifndef TORA_RANGE_H_
#define TORA_RANGE_H_

#include "../value.h"
#include "../shared_ptr.h"
#include "int.h"

namespace tora {

class RangeValue: public Value {
public:
    explicit RangeValue(const SharedPtr<IntValue> &l, const SharedPtr<IntValue> &r);

    const SharedPtr<IntValue> &left() {
        return boost::get<RangeImpl>(value_).left_;
    }
    const SharedPtr<IntValue> &right() {
        return boost::get<RangeImpl>(value_).right_;
    }

    class Iterator : public Value {
    public:
        int counter;
        SharedPtr<RangeValue> parent;
        Iterator(SharedPtr<RangeValue> parent_) : Value(VALUE_TYPE_RANGE_ITERATOR) {
            parent  = parent_;
            counter = parent->left()->int_value();
        }
    };
};

};

#endif // TORA_RANGE_H_
