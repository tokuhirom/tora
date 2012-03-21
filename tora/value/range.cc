#include "range.h"

using namespace tora;


RangeValue::RangeValue(SharedPtr<IntValue> l, SharedPtr<IntValue> r) : Value(VALUE_TYPE_RANGE) {
    left = l;
    right = r;
}
