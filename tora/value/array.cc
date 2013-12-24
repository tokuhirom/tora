#include "../value.h"
#include "array.h"
#include "str.h"
#include <algorithm>

using namespace tora;

/*
 * Default order of sort() function should be string comparision order.
 * It's same as Perl5.
 */
class LessValue {
public:
    bool operator()(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs) const {
        return lhs->to_s()->str_value() < rhs->to_s()->str_value();
    }
};

ArrayValue::ArrayValue(const ArrayValue & a) : Value(VALUE_TYPE_ARRAY) {
    this->array_value_ = new ArrayImpl(new std::deque<SharedPtr<Value>>());
    // this->values->reserve(a.values->size());
    std::copy(a.VAL()->begin(), a.VAL()->end(), std::back_inserter((*(this->VAL()))));
}

// unstable sort
void ArrayValue::sort() {
    std::sort(VAL()->begin(), VAL()->end(), LessValue());
}
// TODO: Array#stable_sort()

SharedPtr<Value> ArrayValue::get_item(const SharedPtr<Value>& index) {
    int i = index->to_int();
    if (i >= (int)VAL()->size()) {
        return new_undef_value();
    } else {
        return this->at(i);
    }
}

void ArrayValue::set_item(int i, const SharedPtr<Value> &v) {
    if ((int)VAL()->size()-1 < i) {
        for (int j=VAL()->size()-1; j<i-1; j++) {
            this->push_back(new_undef_value());
        }
        VAL()->insert(VAL()->begin()+i, v);
    } else {
        if (1) {
            VAL()->erase(VAL()->begin()+i);
            VAL()->insert(VAL()->begin()+i, v);
        } else {
            *(VAL()->at(i).get()) = *v;
        }
        // VAL()->insert(VAL()->begin()+i, v);
    }
}

const SharedPtr<Value> ArrayValue::reverse() const {
    SharedPtr<ArrayValue> res(new ArrayValue());
    for (auto iter = VAL()->rbegin(); iter!= VAL()->rend(); ++iter) {
        res->push_back(*iter);
    }
    return res.get();
}

