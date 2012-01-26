#include <vector>
#include "value.h"

namespace tora {

class Stack {
    std::vector<Value*> container;
public:
    Value *back() {
        return container.back();
    }
    void pop_back() {
        container.pop_back();
    }
    Value *pop() {
        Value * v = container.back();
        container.pop_back();
        return v;
    }
    void push(Value *v) {
        container.push_back(v);
    }
    Value *at(int i) {
        return container.at(i);
    }
    size_t size() {
        return container.size();
    }
};

};
