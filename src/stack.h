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
    SharedPtr<Value> back() {
        return container.back();
    }
    void pop_back() {
        container.pop_back();
    }
    SharedPtr<Value> pop() {
        assert(container.size() > 0);


        SharedPtr<Value> v(container.back());
        container.pop_back();
        return v;
    }
    void push(SharedPtr<Value> v) {
        container.push_back(v);
    }
    SharedPtr<Value> at(int i) {
        return container.at(i);
    }
    size_t size() {
        return container.size();
    }
    void extract_tuple(SharedPtr<TupleValue> &t);
    void dump() {
        printf("-- stack dump --\n");
        for (size_t i=0; i< container.size(); i++) {
            printf("[%d]\n", i);
            container.at(i)->dump(1);
        }
        printf("----------------\n");
    }
};

};

#endif // TORA_STACK_H_

