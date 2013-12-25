#ifndef TORA_VALUE_DOUBLE_H_
#define TORA_VALUE_DOUBLE_H_

#include "../value.h"

namespace tora {

class DoubleValue : public Value {
 public:
  DoubleValue(double d) : Value(VALUE_TYPE_DOUBLE) { this->double_value_ = d; }
};
};

#endif  // TORA_VALUE_DOUBLE_H_
