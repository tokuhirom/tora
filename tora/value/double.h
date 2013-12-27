#ifndef TORA_VALUE_DOUBLE_H_
#define TORA_VALUE_DOUBLE_H_

#include "../value.h"

namespace tora {

  class MortalDoubleValue : public MortalValue {
    static Value* new_value(double d) {
      Value* v = new Value(VALUE_TYPE_DOUBLE);
      v->double_value_ = d;
      return v;
    }
  public:
    MortalDoubleValue(double d)
      : MortalValue(new_value(d)) { }
  };

};

#endif  // TORA_VALUE_DOUBLE_H_
