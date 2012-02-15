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
    VALUE_TYPE_REGEXP,
    VALUE_TYPE_TUPLE,
    VALUE_TYPE_ARRAY_ITERATOR,
    VALUE_TYPE_EXCEPTION,
} value_type_t;

typedef enum {
    EXCEPTION_TYPE_UNDEF,
    EXCEPTION_TYPE_STOP_ITERATION,
} exception_type_t;

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
    void print_indent(int indent) {
        for (int i=0; i<indent; i++) {
            printf("    ");
        }
    }
public:
    value_type_t value_type;

    void dump() { this->dump(0); }
    virtual void dump(int indent) = 0;
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

    virtual SharedPtr<Value> tora__neg__() {
        printf("%s is not a numeric. You cannot apply unary negative operator.\n", this->type_str());
        abort();
    }

    virtual void set_item(SharedPtr<Value>index, SharedPtr<Value>v) {
        printf("%s is not a container. You cannot set item for this type.\n", this->type_str());
        this->dump();
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
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] IV: %d\n", int_value);
    }
    const char *type_str() { return "int"; }
    SharedPtr<StrValue> to_s();
    SharedPtr<Value> tora__neg__();
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
    void dump(int indent) {
        print_indent(indent);
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
    void dump(int indent) {
        print_indent(indent);
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
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] bool: %s\n", bool_value ? "true" : "false");
    }
    const char *type_str() { return "bool"; }
    SharedPtr<StrValue>to_s();
};

class StrValue: public Value {
public:
    std::string str_value;
    StrValue(): Value() {
        this->value_type = VALUE_TYPE_STR;
    }
    StrValue(const char *str): Value() {
        this->value_type = VALUE_TYPE_STR;
        this->str_value = str;
    }
    StrValue(std::string &str): Value() {
        this->value_type = VALUE_TYPE_STR;
        this->str_value = str;
    }
    ~StrValue();
    void set_str(const char*s) {
        str_value = s;
    }
    int length();
    void set_str(std::string s) {
        str_value = s;
    }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] str: %s\n", str_value.c_str());
    }
    const char *type_str() { return "str"; }
    SharedPtr<StrValue> to_s() {
        return this;
    }
};

class ExceptionValue : public Value {
public:
    exception_type_t exception_type;
    ExceptionValue(exception_type_t e) : exception_type(e) {
        this->value_type = VALUE_TYPE_EXCEPTION;
    }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] exception(%d)\n", exception_type);
    }
    const char *type_str() { return "exception"; }
};

};

#endif // VALUE_H_
