#include "stack.h"
#include "value/tuple.h"

using namespace tora;

void Stack::extract_tuple(SharedPtr<TupleValue> &t) {
    int tuple_size = t->size();
    for (int i=0; i<tuple_size; i++) {
        this->push(t->at(i));
    }
}

void Stack::resize(size_t count) {
    assert(count >= 0);
    while (count < size_) {
        pop_back();
    }
}

