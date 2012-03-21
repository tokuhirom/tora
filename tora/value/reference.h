#ifndef TORA_VALUE_REFERENCE_H_
#define TORA_VALUE_REFERENCE_H_

#include "../value.h"

namespace tora {

class ReferenceValue: public Value {
private:
    Value* value_;
public:
    ReferenceValue(Value * v): Value(VALUE_TYPE_REFERENCE) {
        v->retain();
        this->value_ = v;
    }
    ~ReferenceValue() {
        this->value_->release();
        this->value_ = NULL;
    }
    Value *value() const { return value_; }
};

};

#endif // TORA_VALUE_REFERENCE_H_
