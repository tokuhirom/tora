#ifndef TORA_ARRAY_H_
#define TORA_ARRAY_H_

#include "value.h"

namespace tora {

class ArrayValue: public Value {
public:
    std::vector<SharedPtr<Value>> *values;
    ArrayValue() : Value() {
        this->value_type = VALUE_TYPE_ARRAY;
        this->values = new std::vector<SharedPtr<Value>>;
    }
    ~ArrayValue() {
        delete values;
    }
    // retain before push
    void push(SharedPtr<Value> v) {
        this->values->push_back(v);
    }
    size_t size() {
        return this->values->size();
    }
    // release after pop by your hand
    SharedPtr<Value> pop() {
        SharedPtr<Value> v = this->values->back();
        this->values->pop_back();
        return v;
    }
    SharedPtr<Value>at(int i) {
        return this->values->at(i);
    }
    void dump() {
        printf("[dump] array:\n");
        for (size_t i=0; i<values->size(); i++) {
            printf("[%d] ", i);
            values->at(i)->dump();
        }
    }
    SharedPtr<Value>get_item(SharedPtr<Value>index);
    void set_item(SharedPtr<Value>index, SharedPtr<Value>v);
    const char *type_str() { return "array"; }
};

};

#endif // TORA_ARRAY_H_
