#ifndef TORA_RANGE_H_
#define TORA_RANGE_H_

#include "../value.h"

namespace tora {

tra_int range_right(Value* v);
tra_int range_left(Value* v);
class MortalRangeValue : public MortalValue {
 public:
  MortalRangeValue(tra_int l, tra_int r);
};
void range_free(Value* v);

class MortalRangeIteratorValue : public MortalValue {
  static Value* new_value(Value* a);

 public:
  MortalRangeIteratorValue(Value* a)
    : MortalValue(new_value(a)) { }
};

bool range_iter_finished(Value* v);
void range_iter_next(Value* v);
tra_int range_iter_get(Value* v);

};

#endif  // TORA_RANGE_H_
