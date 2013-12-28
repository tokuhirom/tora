#ifndef TORA_VALUE_H_
#define TORA_VALUE_H_

#include <sstream>
#include <cstdio>
#include <cassert>

#include "tora.h"
#include "shared_ptr.h"
#include "exception.h"

namespace tora {

typedef int tra_int;

typedef enum {
  VALUE_TYPE_UNDEF = 0,
  VALUE_TYPE_INT = 1,
  VALUE_TYPE_BOOL = 2,
  VALUE_TYPE_STR = 3,
  VALUE_TYPE_CODE = 4,
  VALUE_TYPE_ARRAY = 5,
  VALUE_TYPE_DOUBLE = 6,
  VALUE_TYPE_REGEXP,
  VALUE_TYPE_TUPLE,
  VALUE_TYPE_FILE,
  VALUE_TYPE_RANGE,
  VALUE_TYPE_ARRAY_ITERATOR,
  VALUE_TYPE_RANGE_ITERATOR,
  VALUE_TYPE_EXCEPTION,
  VALUE_TYPE_SYMBOL,
  VALUE_TYPE_HASH,
  VALUE_TYPE_HASH_ITERATOR,
  VALUE_TYPE_OBJECT,
  VALUE_TYPE_CLASS,
  VALUE_TYPE_POINTER,
  VALUE_TYPE_BYTES,
  VALUE_TYPE_REFERENCE,
  VALUE_TYPE_FILE_PACKAGE,
} value_type_t;

class Value;
class IntValue;
class DoubleValue;
class MortalValue;

class CallbackFunction;

typedef std::string StringImpl;

// TODO: remove virtual from this class for performance.
/**
 * The value class
 */
class Value {
 protected:
  int refcnt_;

 public:
  int refcnt() const {
    assert(refcnt_ >= 0);
    return refcnt_;
  }
  void release();
  void retain() {
    assert(refcnt_ >= 0);
    ++refcnt_;
  }

 protected:
  virtual ~Value();
  Value(const Value&) = delete;

 public:
  Value(value_type_t t) : refcnt_(1), value_type(t) {}
  explicit Value(value_type_t t, void* p)
    : refcnt_(1),
    ptr_value_(p),
    value_type(t) {}
  union {
    int int_value_;
    double double_value_;
    bool bool_value_;
    ID id_value_;
    void* ptr_value_;

    // TODO: remove me.
    std::string* str_value_;

    Value* value_value_;
  };
  value_type_t value_type;
  Value& operator=(const Value& v);

  StringImpl to_s();
  int to_int();
  double to_double() const;
  bool to_bool() const;

  template <class Y>
  Y* upcast() {
    return static_cast<Y*>(this);
  }

  // GET type name in const char*
  const char* type_str() const;

  ID object_package_id() const;

  bool is_exception() const { return value_type == VALUE_TYPE_EXCEPTION; }
};
class SharedValue {
  Value* v_;

 public:
  SharedValue(Value* v) : v_(v) {
    if (v_) {
      v_->retain();
    }
  }
  SharedValue() : v_(NULL) {}
  ~SharedValue();
  SharedValue(const SharedValue& sv) : v_(sv.get()) {
    if (v_) {
      v_->retain();
    }
  }
  SharedValue(const MortalValue& sv);
  SharedValue& operator=(Value* rhs) {
    assert(rhs);
    rhs->retain();
    v_ = rhs;
    return *this;
  }
  Value& operator*() {
    assert(v_);
    return *v_;
  }
  Value* operator->() {
    assert(v_);
    return v_;
  }
  Value* get() const { return v_; }
  void reset(Value* v) {
    if (v_) {
      v_->release();
    }
    assert(v);
    v_ = v;
    v_->retain();
  }
};

static value_type_t type(const Value& v) { return v.value_type; }

static value_type_t type(const Value* v) { return v->value_type; }

static int get_int_value(const Value& v) {
  assert(type(v) == VALUE_TYPE_INT);
  return v.int_value_;
}

static void set_int_value(Value* v, int i) {
  v->value_type = VALUE_TYPE_INT;
  v->int_value_ = i;
}

static void set_int_value(Value& v, int i) { return set_int_value(&v, i); }

static double get_double_value(const Value* v) {
  assert(type(v) == VALUE_TYPE_DOUBLE);
  return v->double_value_;
}

static void* get_ptr_value(const Value* v) { return v->ptr_value_; }

static void* get_ptr_value(const SharedPtr<Value>& v) {
  return get_ptr_value(v.get());
}

static std::string* get_str_value(const Value* v) {
  assert(type(v) == VALUE_TYPE_STR);
  return static_cast<std::string*>(v->ptr_value_);
}

static Value* new_str_value(const StringImpl& s) {
  Value* v = new Value(VALUE_TYPE_STR);
  v->ptr_value_ = new std::string(s);
  return v;
}

static StringImpl* get_str_value(const SharedPtr<Value>& v) {
  // remove me
  return get_str_value(v.get());
}

static int get_int_value(const SharedPtr<Value>& v) {
  // REMOVE ME.
  return get_int_value(*v);
}

static Value* new_undef_value() { return new Value(VALUE_TYPE_UNDEF); }
std::string value_to_bytes(Value* v);

/**
 * Hold local variable.
 *
 * This class does not retain the value.
 * But release it.
 */
class MortalValue {
 protected:
  Value* v_;

 public:
  MortalValue(Value* v) : v_(v) {}
  MortalValue(const MortalValue& mv) = delete;
  ~MortalValue() {
    assert(v_);
    v_->release();
  }
  Value& operator*() const { return *v_; }
  Value* operator->() const {
    assert(v_ != NULL);
    return v_;
  }
  Value* get() const { return v_; }
};
/**
 * MortalStrValue
 *
 * Shorthand to create mortal string value.
 */
class MortalStrValue : public MortalValue {
 public:
  MortalStrValue(const std::string& s) : MortalValue(new_str_value(s)) {}
};
/**
 * MortalStrValue
 *
 * Shorthand to create mortal undef value.
 */
class MortalUndefValue : public MortalValue {
 public:
  MortalUndefValue() : MortalValue(new_undef_value()) {}
};
};

#endif  // TORA_VALUE_H_
