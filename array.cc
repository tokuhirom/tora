#include "value.h"
#include "array.h"

using namespace tora;

Value *ArrayValue::get_item(Value *index) {
    IntValue *iv = index->to_i();
    int i = iv->int_value;
    iv->release();
    if (i>this->values->size()) {
        fprintf(stderr, "IndexError: %d, %d\n", i, this->values->size());
        abort();
    } else {
        return this->at(i);
    }
}

void ArrayValue::set_item(Value *index, Value *v) {
    DBG("SET!!! %d\n", this->values->size());
    IntValuePtr iv(index->to_i());
    int i = iv->int_value;
    DBG("XXX %d, %d\n", this->values->size(), i);
    if (this->values->size()-1 < i) {
        DBG("XXY %d, %d\n", this->values->size(), i);
        for (int j=this->values->size()-1; j<i-1; j++) {
        DBG("put undef %d\n", j);
            this->values->push_back(UndefValue::instance());
        }
    }
    this->values->insert(this->values->begin()+i, v);
    // (*(this->values))[i] = v;
    this->dump();
}

