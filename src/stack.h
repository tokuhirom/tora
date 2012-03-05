#ifndef TORA_STACK_H_
#define TORA_STACK_H_

#include <vector>
#include <new>
#include <stdlib.h>
#include "value.h"
#include "shared_ptr.h"

namespace tora {

class Value;
class TupleValue;

class Stack {
    size_t capacity_;
    size_t size_;
    Value ** data_;
public:
    Stack(size_t initial_capacity) : capacity_(initial_capacity), size_(0) {
        assert(initial_capacity > 0);
        data_ = (Value**)malloc(sizeof(Value*)*initial_capacity);
        if (!data_) {
            throw std::bad_alloc();
        }
    }
    ~Stack() {
        while (size()) {
            pop_back();
        }
        free(data_);
    }
    Value * back() const {
        assert(size_>0);
        return data_[size_-1];
    }
    Value* top() const {
        assert(size_>0);
        return data_[size_-1];
    }
    void pop_back() {
        assert(size_>0);
        data_[size_-1]->release();
        --size_;
    }
    SharedPtr<Value> pop() {
        assert(size_ > 0);

        SharedPtr<Value> v(back());
        pop_back();
        return v;
    }
    void push(Value* v) {
        assert(v);

        // extend
        if (capacity_ < size_+1) {
            size_t new_capacity = capacity_*2;
            Value** new_data = (Value**)realloc(data_, sizeof(Value*)*new_capacity);
            if (new_data) {
                data_ = new_data;
                capacity_ = new_capacity;
            } else {
                throw std::bad_alloc();
            }
        }

        data_[size_++] = v;
        v->retain();
    }
    void push(const SharedPtr<Value> & v) {
        push(v.get());
    }
    void resize(size_t count);
    Value * at(size_t i) {
        assert(i >= 0);
        assert(i < size_);
        return data_[i];
    }
    void set(int i, const SharedPtr<Value>& rvalue) {
        assert(i >= 0);
        assert(i < size_);

        data_[i] = rvalue.get();
        data_[i]->retain();
    }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    void extract_tuple(SharedPtr<TupleValue> &t);
    void dump() const {
        printf("-- stack dump --\n");
        for (size_t i=0; i< size(); i++) {
            printf("[%zd]\n", i);
            data_[i]->dump(1);
        }
        printf("----------------\n");
    }
};

};

#endif // TORA_STACK_H_

