#include "range.h"

using namespace tora;

RangeValue::RangeValue(const SharedPtr<IntValue> &l, const SharedPtr<IntValue>& r) : Value(VALUE_TYPE_RANGE) {
    range_value_ = new RangeImpl(l.get(), r.get());
}

RangeValue::~RangeValue() {
    delete range_value_;
}

