#ifndef TORA_VALUE_INT_H_
#define TORA_VALUE_INT_H_

#include "../value.h"

namespace tora {

class MortalIntValue : public MortalValue {
  static Value* new_value(tra_int i) {
    Value* v = new Value(VALUE_TYPE_INT);
    v->int_value_ = i;
    return v;
  }

 public:
  MortalIntValue(tra_int i) : MortalValue(new_value(i)) {}
};

};

#endif  // TORA_VALUE_INT_H_
