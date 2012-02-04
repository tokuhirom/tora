#ifndef TORA_ARRAY_H_
#define TORA_ARRAY_H_

#include "value.h"

namespace tora {

class ArrayValue: public Value {
public:
    std::vector<Value*> *values;
    ArrayValue() : Value() {
        this->value_type = VALUE_TYPE_ARRAY;
        this->values = new std::vector<Value*>;
    }
    ~ArrayValue() {
        delete values;
    }
    // retain before push
    void push(Value *v) {
        this->values->push_back(v);
    }
    size_t size() {
        return this->values->size();
    }
    // release after pop by your hand
    Value* pop() {
        Value *v = this->values->back();
        this->values->pop_back();
        return v;
    }
    Value *at(int i) {
        return this->values->at(i);
    }
    void dump() {
        printf("[dump] array:\n");
        for (size_t i=0; i<values->size(); i++) {
            printf("[%d] ", i);
            values->at(i)->dump();
        }
    }
    Value *get_item(Value *index);
    void set_item(Value *index, Value *v);
    const char *type_str() { return "array"; }
};

};

#endif // TORA_ARRAY_H_
