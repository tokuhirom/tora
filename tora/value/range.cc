#include "range.h"

using namespace tora;

RangeValue::RangeValue(const SharedPtr<IntValue> &l, const SharedPtr<IntValue>& r) : Value(VALUE_TYPE_RANGE) {
    value_ = RangeImpl(l.get(), r.get());
}
