#ifndef VALUE_H_
#define VALUE_H_

#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <vector>
#include <map>
#include <memory>
#include <deque>

#include <boost/variant.hpp>

#include "tora.h"
#include "shared_ptr.h"
#include "prim.h"
#include "util.h"
#include "op_array.h"

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
    VALUE_TYPE_POINTER,
    VALUE_TYPE_BYTES,
    VALUE_TYPE_REFERENCE,
} value_type_t;

class Value;
class IntValue;
class DoubleValue;
class StrValue;
class BoolValue;
class RangeValue;

class CallbackFunction;
class OPArray;

class RangeImpl {
    friend class RangeValue;
protected:
    SharedPtr<IntValue> left_;
    SharedPtr<IntValue> right_;
    RangeImpl(IntValue* l, IntValue *r) : left_(l), right_(r) {
    }
};

class ObjectImpl {
    friend class ObjectValue;
protected:
    VM * vm_;
    ID package_id_;
    bool destroyed_;
    SharedPtr<Value> data_;
    ObjectImpl(VM *vm, ID pkgid, const SharedPtr<Value>& d)
        : vm_(vm)
        , package_id_(pkgid)
        , destroyed_(false)
        , data_(d)
        { }
};

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
    typedef std::deque<SharedPtr<Value>> ArrayImpl;
    typedef std::map<std::string, SharedPtr<Value> > HashImpl;
    typedef boost::variant<
        int,
        double,
        bool,
        ID,
        std::string,
        RangeImpl,
        ArrayImpl,
        HashImpl,
        void*,
        Value*,
        FILE *,
        ObjectImpl
    > any_t;

    Value(value_type_t t) : refcnt(0), value_type(t) { }
    Value(value_type_t t, any_t dat) : refcnt(0), value_(dat), value_type(t) { }
    virtual ~Value() { }
    Value(const Value&) = delete;

    any_t value_;
public:
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

};

#include "value/undef.h"
#include "value/str.h"
#include "value/bytes.h"
#include "value/bool.h"
#include "value/int.h"
#include "value/double.h"
#include "value/exception.h"

#endif // VALUE_H_
