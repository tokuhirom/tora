#ifndef VALUE_H_
#define VALUE_H_

#include "tora.h"
#include "shared_ptr.h"
#include "prim.h"
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
    VALUE_TYPE_DOUBLE = 6,
} value_type_t;

class CodeValue;
class IntValue;
class DoubleValue;
class StrValue;
class BoolValue;
class ArrayValue;

/**
 * The value class
 */
class Value : public Prim {
protected:
    Value() : Prim() { }
    virtual ~Value() { }
public:
    value_type_t value_type;

    CodeValue* to_code() {
        if (this->value_type == VALUE_TYPE_CODE) {
            return (CodeValue*)this;
        } else {
            printf("[BUG] %d is not a code value\n", this->value_type);
            this->dump();
            exit(1); // BUG
        }
    }
    virtual void dump() = 0;
    // TODO: rename to as_str
    virtual SharedPtr<StrValue> to_s();
    // TODO: rename to as_int
    IntValue *to_i();
    // TODO: rename to as_bool
    SharedPtr<Value> to_b();
    bool is_numeric() {
        return this->value_type == VALUE_TYPE_INT;
    }

    template<class Y>
    Y* upcast() {
        return dynamic_cast<Y*>(&(*(this)));
    }

    // GET type name in const char*
    virtual const char *type_str() = 0;

    virtual Value* tora__neg__() {
        printf("%s is not a numeric. You cannot apply unary negative operator.\n", this->type_str());
        abort();
    }

    virtual void set_item(SharedPtr<Value>index, SharedPtr<Value>v) {
        printf("%s is not a container. You cannot set item for this type.\n", this->type_str());
        abort();
    }
    virtual SharedPtr<Value> get_item(SharedPtr<Value> index) {
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
    ~IntValue() { }
    void dump() {
        printf("[dump] IV: %d\n", int_value);
    }
    const char *type_str() { return "int"; }
    SharedPtr<StrValue> to_s();
    Value* tora__neg__();
    void tora__incr__() {
        this->int_value++;
    }
};

class DoubleValue: public Value {
public:
    double  double_value;
    DoubleValue(double d): Value() {
        this->value_type = VALUE_TYPE_DOUBLE;
        this->double_value = d;
    }
    void dump() {
        printf("[dump] NV: %lf\n", double_value);
    }
    const char *type_str() { return "double"; }
    SharedPtr<StrValue> to_s();
};

class UndefValue: public Value {
private:
    UndefValue(): Value() {
        this->value_type = VALUE_TYPE_UNDEF;
    }
public:
    static UndefValue *instance() {
        return new UndefValue();
    }
    void dump() {
        printf("[dump] undef\n");
    }
    const char *type_str() { return "undef"; }
    SharedPtr<StrValue> to_s();
};

// This is singleton
class BoolValue: public Value {
public:
    BoolValue(bool b): Value() {
        this->value_type = VALUE_TYPE_BOOL;
        this->bool_value = b;
    }
    bool bool_value;
    static SharedPtr<BoolValue> true_instance() {
        return new BoolValue(true);
    }
    static SharedPtr<BoolValue> false_instance() {
        return new BoolValue(false);
    }
    static SharedPtr<BoolValue> instance(bool b) {
        return b ? BoolValue::true_instance() : BoolValue::false_instance();
    }
    void dump() {
        printf("[dump] bool: %s\n", bool_value ? "true" : "false");
    }
    const char *type_str() { return "bool"; }
    SharedPtr<StrValue>to_s();
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
    ~StrValue();
    void set_str(const char*s) {
        str_value = s;
    }
    void dump() {
        printf("[dump] str: %s\n", str_value);
    }
    const char *type_str() { return "str"; }
    SharedPtr<StrValue> to_s() {
        return this;
    }
};

class RangeValue: public Value {
public:
    SharedPtr<IntValue> left;
    SharedPtr<IntValue> right;

    RangeValue(SharedPtr<IntValue> l, SharedPtr<IntValue> r) {
        left = l;
        right = r;
    }
    void dump() {
        printf("[dump] range: %d..%d\n", left->int_value, right->int_value);
    }
    SharedPtr<StrValue> to_s();
    const char *type_str() { return "range"; }
};

typedef SharedPtr<Value>    ValuePtr;
typedef SharedPtr<IntValue> IntValuePtr;
typedef SharedPtr<StrValue> StrValuePtr;

};

#include "array.h"

#endif // VALUE_H_
