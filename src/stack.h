#ifndef TORA_STACK_H_
#define TORA_STACK_H_

#include <vector>
#include "value.h"
#include "value/tuple.h"
#include "shared_ptr.h"

namespace tora {

class Value;

class Stack {
    std::vector< SharedPtr<Value>> container;
public:
    const SharedPtr<Value>& back() {
        return container.back();
    }
    const SharedPtr<Value>& top() {
        return container.back();
    }
    void pop_back() {
        container.pop_back();
    }
    // bit slow... i want to deprecate this method.
    SharedPtr<Value> pop() {
        assert(container.size() > 0);

        SharedPtr<Value> v(container.back());
        container.pop_back();
        return v;
    }
    void push(Value* v) {
        container.push_back(v);
    }
    void push(const SharedPtr<Value> & v) {
        container.push_back(v);
    }
    SharedPtr<Value> at(int i) {
        return container.at(i);
    }
    const SharedPtr<Value>& at(int i) const {
        return container.at(i);
    }
    size_t size() {
        return container.size();
    }
    void extract_tuple(SharedPtr<TupleValue> &t);
    void dump() {
        printf("-- stack dump --\n");
        for (size_t i=0; i< container.size(); i++) {
            printf("[%zd]\n", i);
            container.at(i)->dump(1);
        }
        printf("----------------\n");
    }
};

};

#endif // TORA_STACK_H_

