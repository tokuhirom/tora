#ifndef TORA_TUPLE_H_
#define TORA_TUPLE_H_

#include "../value.h"

namespace tora {

/**
 * Current implementation of tuple value is inherited from array value.
 * I will make it as more flexible and fast implementation later.
 */
class TupleValue : public ArrayValue {
public:
    TupleValue() : ArrayValue() {
    }
    ~TupleValue() { }
    void set_item(SharedPtr<Value>index, SharedPtr<Value>v) {
        printf("tuple is a read only value.\n");
        abort(); // TODO throw exception
    }
    const char *type_str() { return "tuple"; }
};

};

#endif // TORA_TUPLE_H_

