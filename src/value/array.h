#ifndef TORA_ARRAY_H_
#define TORA_ARRAY_H_

#include <deque>
#include "../value.h"

namespace tora {

class ArrayValue: public Value {
public:
    std::deque<SharedPtr<Value>> *values;
    ArrayValue() : Value(VALUE_TYPE_ARRAY) {
        this->values = new std::deque<SharedPtr<Value>>;
    }
    ArrayValue(const ArrayValue & a);
    ~ArrayValue() {
        delete values;
    }
    void sort();

    // retain before push
    void push(Value *v) {
        this->values->push_back(v);
    }
    void push(const SharedPtr<Value> &v) {
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
        printf("[dump] %s(%zd):\n", this->type_str(), values->size());
        for (size_t i=0; i<values->size(); i++) {
            print_indent(indent+1);
            printf("[%zd] ", i);
            values->at(i)->dump(indent+1);
        }
    }
    SharedPtr<Value> get_item(const SharedPtr<Value> &index);
    Value* set_item(const SharedPtr<Value>& index, const SharedPtr<Value> &v);
    virtual const char *type_str() { return "Array"; }

    class iterator : public Value {
    public:
        int counter;
        SharedPtr<ArrayValue> parent;
        iterator() : Value(VALUE_TYPE_ARRAY_ITERATOR), counter(0) {
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
