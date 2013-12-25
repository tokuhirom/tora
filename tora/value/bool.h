#ifndef TORA_VALUE_BOOL_H_
#define TORA_VALUE_BOOL_H_

#include "../value.h"

namespace tora {

class MortalTrueValue : public MortalValue {
  static Value* new_true_value() {
    Value* v = new Value(VALUE_TYPE_BOOL);
    v->bool_value_ = true;
    return v;
  }

 public:
  MortalTrueValue() : MortalValue(new_true_value()) {}
};

class MortalFalseValue : public MortalValue {
  static Value* new_false_value() {
    Value* v = new Value(VALUE_TYPE_BOOL);
    v->bool_value_ = false;
    return v;
  }

 public:
  MortalFalseValue() : MortalValue(new_false_value()) {}
};

static bool get_bool_value(const Value* v) {
  assert(type(v) == VALUE_TYPE_BOOL);
  return v->bool_value_;
}
};

#endif  // TORA_VALUE_BOOL_H_
