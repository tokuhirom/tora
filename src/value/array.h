#ifndef TORA_ARRAY_H_
#define TORA_ARRAY_H_

#include <deque>
#include "../value.h"

namespace tora {

class ArrayValue: public Value {
public:
    typedef std::deque<SharedPtr<Value>>::iterator iterator2;

    std::deque<SharedPtr<Value>> *values;
    ArrayValue() : Value(VALUE_TYPE_ARRAY) {
        this->values = new std::deque<SharedPtr<Value>>;
    }
    ArrayValue(const ArrayValue & a);
    ~ArrayValue() {
        delete values;
    }
    void sort();

    iterator2 begin() { return values->begin(); }
    iterator2 end()   { return values->end();   }

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
    SharedPtr<Value> get_item(const SharedPtr<Value> &index);
    Value* set_item(const SharedPtr<Value>& index, const SharedPtr<Value> &v);
    virtual const char *type_str() { return "Array"; }

    class iterator : public Value {
    public:
        int counter;
        SharedPtr<ArrayValue> parent;
        iterator() : Value(VALUE_TYPE_ARRAY_ITERATOR), counter(0) {
        }
        const char *type_str() { return "array_iterator"; }
    };
};

};

#endif // TORA_ARRAY_H_
