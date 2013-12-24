#ifndef TORA_VALUE_H_
#define TORA_VALUE_H_

#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <vector>
#include <map>
#include <memory>
#include <deque>

#include "tora.h"
#include "shared_ptr.h"
#include "exception.h"

namespace tora {

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
class StrValue;
class BoolValue;
class RangeValue;
class ObjectImpl;
class ClassImpl;
class FilePackageImpl;

class CallbackFunction;

class RangeImpl {
    friend class RangeValue;
protected:
    SharedPtr<IntValue> left_;
    SharedPtr<IntValue> right_;
    RangeImpl(IntValue* l, IntValue *r) : left_(l), right_(r) {
    }
};

class ExceptionImpl {
    friend class ExceptionValue;
    friend class ErrnoExceptionValue;
    friend class ArgumentExceptionValue;
protected:
    exception_type_t type_;
    int errno_;
    std::string message_;
    explicit ExceptionImpl(exception_type_t type) :type_(type) {
    }
    explicit ExceptionImpl(int err, exception_type_t type=EXCEPTION_TYPE_ERRNO) :type_(type), errno_(err) {
    }
    ExceptionImpl(const std::string &msg, exception_type_t type) :type_(type), message_(msg) {
    }
};

typedef std::shared_ptr<std::deque<SharedPtr<Value>>> ArrayImpl;
typedef std::map<std::string, SharedPtr<Value> > HashImpl;
typedef std::string StringImpl;

// TODO: remove virtual from this class for performance.
/**
 * The value class
 */
class Value {
public:
    int refcnt;
    virtual void release() {
        --refcnt;
        if (refcnt == 0) {
            delete this;
        }
    }
    void retain() {
        assert(refcnt >= 0);
        ++refcnt;
    }
protected:

    Value(value_type_t t) : refcnt(0), value_type(t) { }
    virtual ~Value() { }
    Value(const Value&) = delete;
public:
    union {
        int int_value_;
        double double_value_;
        bool bool_value_;
        ID id_value_;

        void * ptr_value_;
        StringImpl* str_value_;
        RangeImpl* range_value_;
        ArrayImpl* array_value_;
        HashImpl*hash_value_;
        Value * value_value_;
        FILE *file_value_;
        ObjectImpl* object_value_;
        ClassImpl* class_value_;
        ExceptionImpl* exception_value_;
        FilePackageImpl* file_package_value_;
    };
    value_type_t value_type;
    Value& operator=(const Value&v);

    SharedPtr<StrValue> to_s();
    int to_int() const;
    double to_double() const;
    bool to_bool() const;

    template<class Y>
    Y* upcast() {
        return static_cast<Y*>(this);
    }

    // GET type name in const char*
    const char *type_str() const;

    ID object_package_id() const;

    bool is_exception() const {
        return value_type == VALUE_TYPE_EXCEPTION;
    }
};

static value_type_t type(const Value &v)
{
  return v.value_type;
}


static int get_int_value(const Value& v)
{
  assert(type(v) == VALUE_TYPE_INT);
  return v.int_value_;
}

static void set_int_value(Value* v, int i)
{
  v->value_type = VALUE_TYPE_INT;
  v->int_value_ = i;
}

static void set_int_value(Value& v, int i)
{
  return set_int_value(&v, i);
}

static double get_double_value(const Value& v)
{
  assert(type(v) == VALUE_TYPE_DOUBLE);
  return v.double_value_;
}

static double get_double_value(const SharedPtr<Value> v)
{
  // REMOVE ME.
  return get_double_value(*v);
}

static int get_int_value(const SharedPtr<Value>& v)
{
  // REMOVE ME.
  return get_int_value(*v);
}

};

#include "value/undef.h"
#include "value/str.h"
#include "value/bool.h"
#include "value/int.h"
#include "value/double.h"
#include "value/exception.h"

#endif // TORA_VALUE_H_
