#ifndef TORA_ARRAY_H_
#define TORA_ARRAY_H_

#include <deque>
#include "../value.h"

namespace tora {

  class MortalArrayIteratorValue : public MortalValue {
    static Value* new_value(Value* a);
  public:
    MortalArrayIteratorValue(Value* a)
      : MortalValue(new_value(a)) { }
  };

  class MortalArrayValue : public MortalValue {
    static Value* new_value();
  public:
    MortalArrayValue()
      : MortalValue(new_value()) { }
  };

  class MortalTupleValue : public MortalValue {
    static Value* new_value();
  public:
    MortalTupleValue()
      : MortalValue(new_value()) { }
  };

  Value * array_iter_get(Value *iter);
  bool array_iter_finished(Value *iter);
  void array_iter_next(Value *iter);

  SharedValue array_reverse(const Value* v);
  void array_push_back(Value* ary, Value* v);
  void array_push_front(Value* ary, Value* v);
  void array_pop_front(Value* ary);
  void array_set_item(Value* ary, tra_int i, Value* v);
  Value* array_get_item(const Value *self, tra_int i);
  void array_pop_back(Value* self);
  Value* array_back(const Value* self);
  tra_int array_size(const Value* self);
  void REMOVE_ME_array_copy(Value* a, const Value *b);
  void array_copy(Value* a, const Value *b);
  SharedValue array_stable_sort(const Value* self);
  void array_free(Value* self);

  // tuple
  Value* tuple_get_item(const Value* self, tra_int i);
  tra_int tuple_size(const Value* self);
  void tuple_push(Value* self, Value* v);

};

#endif // TORA_ARRAY_H_
