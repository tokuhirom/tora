#ifndef TORA_VALUE_POINTER_H_
#define TORA_VALUE_POINTER_H_

#include "../value.h"

namespace tora {

class PointerValue : public Value {
public:
    PointerValue(void * p) : Value(VALUE_TYPE_POINTER, p) {
    }
    ~PointerValue() { }

    void *ptr() {
        return boost::get<void*>(value_);
    }
};

};

#endif // TORA_VALUE_POINTER_H_
