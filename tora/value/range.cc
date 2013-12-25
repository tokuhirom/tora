#include "range.h"

using namespace tora;

RangeValue::RangeValue(tra_int l, tra_int r) : Value(VALUE_TYPE_RANGE) {
    range_value_ = new RangeImpl(l, r);
}

RangeValue::~RangeValue() {
    delete range_value_;
}

