#include <vector>
#include "value.h"
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
        SharedPtr<Value> v = container.back();
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
};

};
