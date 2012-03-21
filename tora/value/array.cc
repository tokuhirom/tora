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
    this->value_ = std::deque<SharedPtr<Value>>();
    // this->values->reserve(a.values->size());
    std::copy(a.VAL().begin(), a.VAL().end(), std::back_inserter(this->VAL()));
}

// unstable sort
void ArrayValue::sort() {
    std::sort(VAL().begin(), VAL().end(), LessValue());
}
// TODO: Array#stable_sort()

SharedPtr<Value> ArrayValue::get_item(const SharedPtr<Value>& index) {
    int i = index->to_int();
    if (i > (int)VAL().size()) {
        return UndefValue::instance();
    } else {
        return this->at(i);
    }
}

Value* ArrayValue::set_item(const SharedPtr<Value>& index, const SharedPtr<Value> &v) {
    int i = index->to_int();
    if ((int)VAL().size()-1 < i) {
        for (int j=VAL().size()-1; j<i-1; j++) {
            this->push_back(UndefValue::instance());
        }
    } else {
        VAL().erase(VAL().begin()+i);
    }
    VAL().insert(VAL().begin()+i, v);
    return UndefValue::instance();
}

