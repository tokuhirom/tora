#ifndef TORA_VALUE_POINTER_H_
#define TORA_VALUE_POINTER_H_

#include "../value.h"

namespace tora {

class PointerValue : public Value {
public:
    PointerValue(void * p) : Value(VALUE_TYPE_POINTER) {
        ptr_value_ = p;
    }
    ~PointerValue() { }

    void *ptr() {
        return ptr_value_;
    }
};

};

#endif // TORA_VALUE_POINTER_H_
