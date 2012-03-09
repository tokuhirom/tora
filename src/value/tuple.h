#ifndef TORA_TUPLE_H_
#define TORA_TUPLE_H_

#include "../value.h"
#include "array.h"

namespace tora {

/**
 * Current implementation of tuple value is inherited from array value.
 * I will make it as more flexible and fast implementation later.
 */
class TupleValue : public ArrayValue {
public:
    TupleValue() : ArrayValue() {
        this->value_type = VALUE_TYPE_TUPLE;
    }
    ~TupleValue() { }
    Value* set_item(SharedPtr<Value>index, SharedPtr<Value>v) {
        printf("tuple is a read only value.\n");
        abort(); // TODO throw exception
    }
};

};

#endif // TORA_TUPLE_H_

