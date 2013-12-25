#include "../value.h"
#include "array.h"
#include <algorithm>

using namespace tora;

namespace tora {
  struct ArrayIteratorImpl {
    int counter;
    SharedValue parent;

    ArrayIteratorImpl(Value* v)
      : counter(0), parent(v) { }
  };

  typedef std::deque<SharedValue> ArrayImpl;

  /**
  * Current implementation of tuple value is inherited from array value.
  * I will make it as more flexible and fast implementation later.
  */
  typedef ArrayImpl TupleImpl;
};

Value* MortalTupleValue::new_value()
{
  Value *v = new Value(VALUE_TYPE_TUPLE);
  v->ptr_value_ = new TupleImpl;
  return v;
}

Value* MortalArrayValue::new_value()
{
  Value *v = new Value(VALUE_TYPE_ARRAY);
  v->ptr_value_ = new ArrayImpl;
  return v;
}

// -- utils --

static ArrayImpl* ary(const Value* v)
{
  assert(type(v) == VALUE_TYPE_ARRAY);
  return static_cast<ArrayImpl*>(get_ptr_value(v));
}

static TupleImpl* tuple(const Value* v)
{
  assert(type(v) == VALUE_TYPE_TUPLE);
  return static_cast<TupleImpl*>(get_ptr_value(v));
}

static ArrayIteratorImpl* ary_iter(const Value* v)
{
  assert(type(v) == VALUE_TYPE_ARRAY_ITERATOR);
  return static_cast<ArrayIteratorImpl*>(get_ptr_value(v));
}

Value* MortalArrayIteratorValue::new_value(Value* a)
{
  Value* v = new Value(VALUE_TYPE_ARRAY_ITERATOR);
  v->ptr_value_ = new ArrayIteratorImpl(a);
  return v;
}

// -- array iter impl --

Value * tora::array_iter_get(Value *iter)
{
  return ary(ary_iter(iter)->parent.get())->at(ary_iter(iter)->counter).get();
}

bool tora::array_iter_finished(Value *iter)
{
  return ary(ary_iter(iter)->parent.get())->size() <= ary_iter(iter)->counter;
}

void tora::array_iter_next(Value* iter)
{
  ary_iter(iter)->counter++;
}

// -- Array --

Value* tora::array_reverse(const Value* v)
{
  MortalArrayValue res;
  // TODO extend res's size first.
  for (tra_int i=0, l=array_size(v); i<l; ++i) {
    array_push_front(res.get(), array_get_item(v, i));
  }
  return res.get();
}

/*
 * Default order of sort() function should be string comparision order.
 * It's same as Perl5.
 */
class LessValue {
public:
    bool operator()(const SharedValue& lhs, const SharedValue& rhs) const {
        return lhs.get()->to_s() < rhs.get()->to_s();
    }
};

// stable sort
Value* tora::array_stable_sort(const Value* self) {
  MortalArrayValue av;
  *(ary(av.get())) = *(ary(self));
  std::stable_sort(ary(av.get())->begin(), ary(av.get())->end(), LessValue());
  return av.get();
}

Value* tora::array_get_item(const Value *self, tra_int i)
{
  if (i >= (int)ary(self)->size()) {
      return new_undef_value();
  } else {
      return ary(self)->at(i).get();
  }
}

void tora::array_set_item(Value* self, tra_int i, Value* v)
{
  ArrayImpl* a = ary(self);
  if ((int)a->size()-1 < i) {
    // Fill undef
    for (int j=a->size()-1; j<i-1; j++) {
      a->push_back(new_undef_value());
    }
    a->insert(a->begin()+i, v);
  } else {
      if (1) {
          a->erase(a->begin()+i);
          a->insert(a->begin()+i, v);
      } else {
          *(a->at(i).get()) = *v;
      }
      // VAL()->insert(VAL()->begin()+i, v);
  }
}

tra_int tora::array_size(const Value* self)
{
  return ary(self)->size();
}

void tora::array_push_front(Value* self, Value* v)
{
  ary(self)->push_front(v);
}

void tora::array_push_back(Value* self, Value* v)
{
  ary(self)->push_back(v);
}

void tora::array_pop_back(Value* self)
{
  ary(self)->pop_back();
}

void tora::array_pop_front(Value* self)
{
  ary(self)->pop_front();
}

// TODO remove me
void tora::REMOVE_ME_array_copy(Value* a, const Value *b)
{
  a->value_type = VALUE_TYPE_ARRAY;
  a->ptr_value_ = new ArrayImpl();
  *(ary(a)) = *(ary(b));
}

void tora::array_copy(Value* a, const Value *b)
{
  assert(type(a) == VALUE_TYPE_ARRAY);
  assert(type(b) == VALUE_TYPE_ARRAY);

  *(ary(a)) = *(ary(b));
}

void tora::array_free(Value* self)
{
  delete ary(self);
  self->ptr_value_ = NULL;
}

Value* tora::array_back(const Value* self)
{
  return ary(self)->back().get();
}

// -- tuple --

/**
 * Return borrowed.
 */
Value* tora::tuple_get_item(const Value* self, tra_int i)
{
  if (i >= (int)tuple(self)->size()) {
    // TODO return unique.
    return new_undef_value();
  } else {
    return tuple(self)->at(i).get();
  }
}

tra_int tora::tuple_size(const Value* self)
{
  return tuple(self)->size();
}

void tora::tuple_push(Value* self, Value* v)
{
  tuple(self)->push_back(v);
}

