#ifndef TORA_ARRAY_H_
#define TORA_ARRAY_H_

#include <deque>
#include "../value.h"

namespace tora {

class ArrayValue: public Value {
private:
    inline ArrayImpl& VAL() {
        return boost::get<ArrayImpl>(this->value_);
    }
    inline const ArrayImpl& VAL() const {
        return boost::get<ArrayImpl>(this->value_);
    }
public:
    typedef ArrayImpl::iterator iterator2;

    ArrayValue() : Value(VALUE_TYPE_ARRAY) {
        this->value_ = std::deque<SharedPtr<Value>>();
    }
    ArrayValue(const ArrayValue & a);
    ~ArrayValue() {
    }
    void sort();

    iterator2 begin() { return VAL().begin(); }
    iterator2 end()   { return VAL().end();   }

    // retain before push
    void push(Value *v) {
        // TODO: remove this method.
        this->push_back(v);
    }
    void push(const SharedPtr<Value> &v) {
        // TODO: remove this method.
        this->push_back(v);
    }
    void push_back(Value *v) {
        VAL().push_back(v);
    }
    void push_back(const SharedPtr<Value> &v) {
        VAL().push_back(v);
    }
    void push_front(Value *v) {
        VAL().push_front(v);
    }
    void push_front(const SharedPtr<Value> &v) {
        VAL().push_front(v);
    }
    void pop_front() {
        VAL().pop_front();
    }
    size_t size() {
        return VAL().size();
    }
    // release after pop by your hand
    SharedPtr<Value> pop() {
        SharedPtr<Value> v = VAL().back();
        VAL().pop_back();
        return v;
    }
    SharedPtr<Value>at(int i) const {
        return VAL().at(i);
    }
    SharedPtr<Value> get_item(const SharedPtr<Value> &index);
    Value* set_item(const SharedPtr<Value>& index, const SharedPtr<Value> &v);

    class iterator : public Value {
    public:
        int counter;
        SharedPtr<ArrayValue> parent;
        iterator() : Value(VALUE_TYPE_ARRAY_ITERATOR), counter(0) {
        }
    };
};

};

#endif // TORA_ARRAY_H_
