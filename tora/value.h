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

#include "tora.h"
#include "shared_ptr.h"
#include "prim.h"
#include "util.h"

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

class IntValue;
class DoubleValue;
class StrValue;
class BoolValue;

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
    union {
        int int_value_;
    };
public:
    value_type_t value_type;
    Value& operator=(const Value&v);

    SharedPtr<StrValue> to_s();
    int to_int();
    double to_double();
    bool to_bool();

    template<class Y>
    Y* upcast() {
        return static_cast<Y*>(this);
    }

    // GET type name in const char*
    const char *type_str() const;

    ID object_package_id();

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
