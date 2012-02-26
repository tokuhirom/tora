#ifndef TORA_VALUE_POINTER_H_
#define TORA_VALUE_POINTER_H_

#include "../value.h"

namespace tora {

class PointerValue : public Value {
    void * ptr_;
public:
    PointerValue(void * p) : Value(VALUE_TYPE_POINTER), ptr_(p) {
    }
    ~PointerValue() { }

    void *ptr() { return ptr_; }

    void dump(int indent) {
        print_indent(indent);
        printf("[dump] Pointer: %X\n", (unsigned int)ptr_);
    }
    const char *type_str() { return "pointer"; }

};

};

#endif // TORA_VALUE_POINTER_H_
