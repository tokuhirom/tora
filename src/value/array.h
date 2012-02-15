#ifndef TORA_ARRAY_H_
#define TORA_ARRAY_H_

#include "../value.h"

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
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] array(%zd):\n", values->size());
        for (size_t i=0; i<values->size(); i++) {
            printf("[%zd] ", i);
            values->at(i)->dump(indent+1);
        }
    }
    SharedPtr<Value>get_item(SharedPtr<Value>index);
    void set_item(SharedPtr<Value>index, SharedPtr<Value>v);
    const char *type_str() { return "array"; }

    class iterator : public Value {
    public:
        int counter;
        SharedPtr<ArrayValue> parent;
        iterator() {
            counter = 0;
            value_type = VALUE_TYPE_ARRAY_ITERATOR;
        }
        void dump(int indent) {
            print_indent(indent);
            printf("[dump] array_iterator(%d):\n", counter);
            parent->dump(indent+1);
        }
        const char *type_str() { return "array_iterator"; }
    };
};

};

#endif // TORA_ARRAY_H_
