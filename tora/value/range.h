#ifndef TORA_RANGE_H_
#define TORA_RANGE_H_

#include "../value.h"
#include "../shared_ptr.h"
#include "int.h"

namespace tora {

class RangeValue: public Value {
public:
    explicit RangeValue(const SharedPtr<IntValue> &l, const SharedPtr<IntValue> &r);
    ~RangeValue();

    const SharedPtr<IntValue> &left() const {
        return range_value_->left_;
    }
    const SharedPtr<IntValue> &right() {
        return range_value_->right_;
    }

    class Iterator : public Value {
    public:
        int counter;
        SharedPtr<RangeValue> parent;
        Iterator(SharedPtr<RangeValue> parent_) : Value(VALUE_TYPE_RANGE_ITERATOR) {
            parent  = parent_;
            counter = get_int_value(*(parent->left()));
        }
    };
};

};

#endif // TORA_RANGE_H_
