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

  Value * array_iter_get(Value *iter);
  bool array_iter_finished(Value *iter);
  void array_iter_next(Value *iter);

class ArrayValue: public Value {
    friend class Value;
protected:
    inline ArrayImpl& VAL() {
        return *(this->array_value_);
    }
    inline const ArrayImpl& VAL() const {
        return *(this->array_value_);
    }
public:
    // rename to iterator
    typedef std::deque<SharedPtr<Value>>::iterator iterator2;
    typedef std::deque<SharedPtr<Value>>::const_iterator const_iterator;

    ArrayValue() : Value(VALUE_TYPE_ARRAY) {
        this->array_value_ = new std::shared_ptr<std::deque<SharedPtr<Value>>>(new std::deque<SharedPtr<Value>>());
    }
    ArrayValue(const ArrayValue & a);
    ~ArrayValue() {
        delete this->array_value_;
    }
    void sort();

    iterator2 begin() { return VAL()->begin(); }
    iterator2 end()   { return VAL()->end();   }
    const_iterator begin() const { return VAL()->begin(); }
    const_iterator end()   const { return VAL()->end();   }

    void pop_back() {
        VAL()->pop_back();
    }
    void push_back(Value *v) {
        VAL()->push_back(v);
    }
    void push_back(const SharedPtr<Value> &v) {
        VAL()->push_back(v);
    }
    void push_front(Value *v) {
        VAL()->push_front(v);
    }
    void push_front(const SharedPtr<Value> &v) {
        VAL()->push_front(v);
    }
    void pop_front() {
        VAL()->pop_front();
    }
    size_t size() const {
        return VAL()->size();
    }
    void resize(size_t n) {
        VAL()->resize(n);
    }
    const SharedPtr<Value>& back() const {
        return VAL()->back();
    }
    SharedPtr<Value>at(int i) const {
        return VAL()->at(i);
    }
    SharedPtr<Value> operator[](int i) const {
        return (*(VAL()))[i];
    }
    SharedPtr<Value> get_item(const SharedPtr<Value> &index);
    void set_item(int i, const SharedPtr<Value> &v);
    void set_item(const SharedPtr<Value>& index, const SharedPtr<Value> &v) {
        this->set_item(index->to_int(), v);
    }
    const SharedPtr<Value> reverse() const;

};

};

#endif // TORA_ARRAY_H_
