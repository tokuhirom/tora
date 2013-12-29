#include "range.h"

using namespace tora;

class RangeImpl {
 protected:
  tra_int left_;
  tra_int right_;

 public:
  RangeImpl(tra_int l, tra_int r) : left_(l), right_(r) {}
  tra_int left() const { return left_; }
  tra_int right() const { return right_; }
};

struct RangeIteratorImpl {
  tra_int counter;
  SharedValue range;
};

static RangeIteratorImpl* range_iter(Value* v) {
  assert(type(v) == VALUE_TYPE_RANGE_ITERATOR);
  return static_cast<RangeIteratorImpl*>(get_ptr_value(v));
}


static RangeImpl* range(Value* v) {
  assert(type(v) == VALUE_TYPE_RANGE);
  return static_cast<RangeImpl*>(get_ptr_value(v));
}

bool tora::range_iter_finished(Value* v) {
  RangeIteratorImpl* iter = range_iter(v);
  RangeImpl* rng = range(iter->range.get());
  return iter->counter > rng->right();
}

void tora::range_iter_next(Value* v) {
  RangeIteratorImpl* iter = range_iter(v);
  iter->counter++;
}

Value* tora::MortalRangeIteratorValue::new_value(tora::Value* v)
{
  RangeIteratorImpl *iter = new RangeIteratorImpl();
  iter->counter = range(v)->left();
  iter->range = v;

  Value * val = new Value(VALUE_TYPE_RANGE_ITERATOR);
  val->ptr_value_ = iter;
  assert(type(val) == VALUE_TYPE_RANGE_ITERATOR);
  return val;
}

tra_int tora::range_iter_get(Value* v) {
  return range_iter(v)->counter;
}

tra_int tora::range_left(Value* v) { return range(v)->left(); }

tra_int tora::range_right(Value* v) { return range(v)->right(); }

static Value* new_range_value(tra_int l, tra_int r) {
  Value* v = new Value(VALUE_TYPE_RANGE);
  v->ptr_value_ = new RangeImpl(l, r);
  return v;
}

tora::MortalRangeValue::MortalRangeValue(tra_int l, tra_int r)
    : MortalValue(new_range_value(l, r)) {}

void tora::range_free(Value* v) { delete range(v); }
