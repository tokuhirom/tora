#ifndef TORA_VALUE_DOUBLE_H_
#define TORA_VALUE_DOUBLE_H_

#include "../value.h"

namespace tora {

class DoubleValue: public Value {
public:
    DoubleValue(double d): Value(VALUE_TYPE_DOUBLE) {
        this->double_value_ = d;
    }
    double double_value() const {
        return this->double_value_;
    }
    void double_value(double n) {
        this->double_value_ = n;
    }
};

};

#endif // TORA_VALUE_DOUBLE_H_
