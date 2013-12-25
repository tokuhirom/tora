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
};

static ArrayImpl* ary(const Value* v)
{
  assert(type(v) == VALUE_TYPE_ARRAY);
  return static_cast<ArrayImpl*>(get_ptr_value(v));
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

Value * tora::array_iter_get(Value *iter)
{
  return (*ary(ary_iter(iter)->parent.get()))->at(ary_iter(iter)->counter).get();
}

bool tora::array_iter_finished(Value *iter)
{
  return (*ary(ary_iter(iter)->parent.get()))->size() <= ary_iter(iter)->counter;
}

void tora::array_iter_next(Value* iter)
{
  ary_iter(iter)->counter++;
}

/*
 * Default order of sort() function should be string comparision order.
 * It's same as Perl5.
 */
class LessValue {
public:
    bool operator()(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs) const {
        return lhs->to_s() < rhs->to_s();
    }
};

ArrayValue::ArrayValue(const ArrayValue & a) : Value(VALUE_TYPE_ARRAY) {
    this->array_value_ = new ArrayImpl(new std::deque<SharedPtr<Value>>());
    // this->values->reserve(a.values->size());
    std::copy(a.VAL()->begin(), a.VAL()->end(), std::back_inserter((*(this->VAL()))));
}

// unstable sort
void ArrayValue::sort() {
    std::sort(VAL()->begin(), VAL()->end(), LessValue());
}
// TODO: Array#stable_sort()

SharedPtr<Value> ArrayValue::get_item(const SharedPtr<Value>& index) {
    int i = index->to_int();
    if (i >= (int)VAL()->size()) {
        return new_undef_value();
    } else {
        return this->at(i);
    }
}

void ArrayValue::set_item(int i, const SharedPtr<Value> &v) {
    if ((int)VAL()->size()-1 < i) {
        for (int j=VAL()->size()-1; j<i-1; j++) {
            this->push_back(new_undef_value());
        }
        VAL()->insert(VAL()->begin()+i, v);
    } else {
        if (1) {
            VAL()->erase(VAL()->begin()+i);
            VAL()->insert(VAL()->begin()+i, v);
        } else {
            *(VAL()->at(i).get()) = *v;
        }
        // VAL()->insert(VAL()->begin()+i, v);
    }
}

const SharedPtr<Value> ArrayValue::reverse() const {
    SharedPtr<ArrayValue> res(new ArrayValue());
    for (auto iter = VAL()->rbegin(); iter!= VAL()->rend(); ++iter) {
        res->push_back(*iter);
    }
    return res.get();
}

