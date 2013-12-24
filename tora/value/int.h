#ifndef TORA_VALUE_INT_H_
#define TORA_VALUE_INT_H_

#include "../value.h"

namespace tora {

class IntValue: public Value {
private:
    const int &VAL() const {
        return this->int_value_;
    }
    int &VAL() {
        return this->int_value_;
    }
public:
    IntValue(int i): Value(VALUE_TYPE_INT) {
        this->int_value_ = i;
    }
    ~IntValue() { }
    // remove?
    void tora__decr__() {
        VAL()--;
    }
    SharedPtr<IntValue> clone() const {
        return new IntValue(VAL());
    }
    // remove?
    void tora__incr__() {
        VAL()++;
    }
};

};

#endif // TORA_VALUE_INT_H_
