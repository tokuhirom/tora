#ifndef TORA_RANGE_H_
#define TORA_RANGE_H_

#include "../value.h"
#include "../shared_ptr.h"
#include "int.h"

namespace tora {

tra_int range_right(Value* v);
tra_int range_left(Value* v);
class MortalRangeValue : public MortalValue {
 public:
  MortalRangeValue(tra_int l, tra_int r);
};
void range_free(Value* v);

class RangeValue {
 public:
  explicit RangeValue(tra_int l, tra_int r);
  class Iterator : public Value {
   public:
    int counter;
    SharedPtr<Value> parent;
    Iterator(SharedPtr<Value> parent_) : Value(VALUE_TYPE_RANGE_ITERATOR) {
      parent = parent_;
      counter = range_left(parent.get());
    }
  };
};
};

#endif  // TORA_RANGE_H_
