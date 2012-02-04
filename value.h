#ifndef VALUE_H_
#define VALUE_H_

#include "tora.h"
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <vector>
#include <map>
#include <memory>

namespace tora {

typedef enum {
    VALUE_TYPE_UNDEF = 0,
    VALUE_TYPE_INT = 1,
    VALUE_TYPE_BOOL = 2,
    VALUE_TYPE_STR = 3,
    VALUE_TYPE_CODE = 4,
    VALUE_TYPE_ARRAY = 5,
} value_type_t;

class CodeValue;
class IntValue;
class StrValue;
class BoolValue;
class ArrayValue;

/**
 * The value class
 */
class Value {
protected:
    Value() {
        refcnt = 1;
    }
public:
    value_type_t value_type;
    int refcnt; // reference count

    ~Value() {
    }
    inline virtual void release() {
        --refcnt;
        if (refcnt == 0) {
            delete this;
        }
    }
    inline virtual void retain() {
        ++refcnt;
    }
    CodeValue* to_code() {
        if (this->value_type == VALUE_TYPE_CODE) {
            return (CodeValue*)this;
        } else {
            printf("[BUG] %d is not a code value\n", this->value_type);
            this->dump();
            exit(1); // BUG
        }
    }
    IntValue* to_int() {
        assert(this->value_type == VALUE_TYPE_INT);
        return (IntValue*)this;
    }
    BoolValue* to_bool() {
        assert(this->value_type == VALUE_TYPE_BOOL);
        return (BoolValue*)this;
    }
    StrValue* to_str() {
        assert(this->value_type == VALUE_TYPE_STR);
        return (StrValue*)this;
    }
    ArrayValue* to_array() {
        assert(this->value_type == VALUE_TYPE_ARRAY);
        return (ArrayValue*)this;
    }
    virtual void dump() = 0;
    // TODO: rename to as_str
    virtual StrValue *to_s();
    // TODO: rename to as_int
    IntValue *to_i();
    // TODO: rename to as_bool
    Value *to_b();
    bool is_numeric() {
        return this->value_type == VALUE_TYPE_INT;
    }

    // GET type name in const char*
    virtual const char *type_str() = 0;

    virtual Value* tora__neg__() {
        printf("%s is not a numeric. You cannot apply unary negative operator.\n", this->type_str());
        abort();
    }

    virtual void set_item(Value *index, Value *v) {
        printf("%s is not a container. You cannot set item for this type.\n", this->type_str());
        abort();
    }
    virtual Value *get_item(Value *index) {
        printf("This is not a container type: %s\n", this->type_str());
        abort();
    }
};

class IntValue: public Value {
public:
    int  int_value;
    IntValue(int i): Value() {
        this->value_type = VALUE_TYPE_INT;
        this->int_value = i;
    }
    void dump() {
        printf("[dump] IV: %d\n", int_value);
    }
    const char *type_str() { return "int"; }
    StrValue *to_s();
    Value* tora__neg__();
};

class UndefValue: public Value {
private:
    UndefValue(): Value() {
        this->value_type = VALUE_TYPE_UNDEF;
    }
    static UndefValue *undef_;
public:
    void release() { } // DO NOT RELEASE
    void retain()  { } // NOP
    static UndefValue *instance() {
        if (!undef_) {
            undef_ = new UndefValue();
        }
        return undef_;
    }
    void dump() {
        printf("[dump] undef\n");
    }
    const char *type_str() { return "undef"; }
    StrValue *to_s();
};

// This is singleton
class BoolValue: public Value {
private:
    BoolValue(bool b): Value() {
        this->value_type = VALUE_TYPE_BOOL;
        this->bool_value = b;
    }
    static BoolValue *true_;
    static BoolValue *false_;
public:
    bool bool_value;
    void release() {
        // DO NOT RELEASE
    }
    void retain() {
        // NOP
    }
    static BoolValue *true_instance() {
        if (!true_) {
            true_ = new BoolValue(true);
        }
        return true_;
    }
    static BoolValue *false_instance() {
        if (!false_) {
            false_ = new BoolValue(false);
        }
        return false_;
    }
    static BoolValue *instance(bool b) {
        return b ? BoolValue::true_instance() : BoolValue::false_instance();
    }
    void dump() {
        printf("[dump] bool: %s\n", bool_value ? "true" : "false");
    }
    const char *type_str() { return "bool"; }
    StrValue *to_s();
};

class StrValue: public Value {
public:
    const char*str_value;
    StrValue(): Value() {
        this->value_type = VALUE_TYPE_STR;
        this->str_value = NULL;
    }
    StrValue(const char *str): Value() {
        this->value_type = VALUE_TYPE_STR;
        this->str_value = str;
    }
    ~StrValue() {
        delete [] str_value;
        this->str_value = NULL;
    }
    void set_str(const char*s) {
        str_value = s;
    }
    void dump() {
        printf("[dump] str: %s\n", str_value);
    }
    const char *type_str() { return "str"; }
    StrValue *to_s() {
        this->retain();
        return this;
    }
};

struct ValueDeleter {
    typedef Value pointer;
    void operator ()(Value* handle) {
        handle->release();
    }
};

typedef std::unique_ptr<Value, ValueDeleter> ValuePtr;
typedef std::unique_ptr<ArrayValue, ValueDeleter> ArrayValuePtr;
typedef std::unique_ptr<IntValue, ValueDeleter> IntValuePtr;
typedef std::unique_ptr<StrValue, ValueDeleter> StrValuePtr;

};

#include "array.h"

#endif // VALUE_H_
