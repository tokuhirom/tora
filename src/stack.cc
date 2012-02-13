#include "stack.h"

using namespace tora;

void Stack::extract_tuple(SharedPtr<TupleValue> &t) {
    int tuple_size = t->size();
    for (int i=0; i<tuple_size; i++) {
        this->push(t->at(i));
    }
}

