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
    VALUE_TYPE_FILE,
    VALUE_TYPE_RANGE,
    VALUE_TYPE_ARRAY_ITERATOR,
    VALUE_TYPE_RANGE_ITERATOR,
    VALUE_TYPE_EXCEPTION,
    VALUE_TYPE_SYMBOL,
    VALUE_TYPE_HASH,
    VALUE_TYPE_HASH_ITERATOR,
    VALUE_TYPE_PACKAGE,
    VALUE_TYPE_PACKAGE_MAP,
    VALUE_TYPE_OBJECT,
    VALUE_TYPE_POINTER,
} value_type_t;

typedef enum {
    EXCEPTION_TYPE_UNDEF,
    EXCEPTION_TYPE_STOP_ITERATION,
    EXCEPTION_TYPE_GENERAL,
} exception_type_t;

class CodeValue;
class IntValue;
class DoubleValue;
class StrValue;
class BoolValue;
class ArrayValue;
class SymbolTable;

// TODO: remove virtual from this class for performance.
/**
 * The value class
 */
class Value : public Prim {
protected:
    Value(value_type_t t) : Prim(), value_type(t) { }
    virtual ~Value() { }
    void print_indent(int indent) {
        for (int i=0; i<indent; i++) {
            printf("    ");
        }
    }
    Value(const Value&) = delete;
public:
    value_type_t value_type;
    Value& operator=(const Value&v);

    void dump() { this->dump(0); }
    void dump(int indent);
    virtual void dump(const SharedPtr<SymbolTable> & symbol_table, int indent) {
        this->dump(indent);
    }
    // TODO: rename to as_str
    virtual SharedPtr<StrValue> to_s();
    Value *to_int();
    // TODO: rename to as_bool
    SharedPtr<BoolValue> to_b();
    bool is_numeric() {
        return this->value_type == VALUE_TYPE_INT;
    }

    template<class Y>
    Y* upcast() {
        return static_cast<Y*>(this);
    }

    // GET type name in const char*
    const char *type_str();

    virtual Value* set_item(SharedPtr<Value>index, SharedPtr<Value>v) {
        printf("%s is not a container. You cannot set item for this type.\n", this->type_str());
        this->dump();
        abort();
    }
    virtual SharedPtr<Value> get_item(SharedPtr<Value> index) {
        printf("This is not a container type: %s\n", this->type_str());
        abort();
    }

    bool is_exception() {
        return value_type == VALUE_TYPE_EXCEPTION;
    }
};

class IntValue: public Value {
public:
    int  int_value;
    IntValue(int i): Value(VALUE_TYPE_INT) {
        this->int_value = i;
    }
    ~IntValue() { }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] IV: %d\n", int_value);
    }
    const char *type_str() { return "int"; }
    SharedPtr<StrValue> to_s();
    void tora__decr__() {
        this->int_value--;
    }
    void tora__incr__() {
        this->int_value++;
    }
    SharedPtr<IntValue> clone() {
        return new IntValue(this->int_value);
    }
};

class DoubleValue: public Value {
public:
    double  double_value;
    DoubleValue(double d): Value(VALUE_TYPE_DOUBLE) {
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
    UndefValue(): Value(VALUE_TYPE_UNDEF) { }
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
    BoolValue(bool b): Value(VALUE_TYPE_BOOL) {
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
    BoolValue* tora__not__() {
        return new BoolValue(!this->bool_value);
    }
};

class StrValue: public Value {
public:
    std::string str_value;
    StrValue(): Value(VALUE_TYPE_STR) {
    }
    StrValue(const char *str): Value(VALUE_TYPE_STR) {
        this->str_value = str;
    }
    StrValue(const std::string str): Value(VALUE_TYPE_STR) {
        this->str_value = str;
    }
    ~StrValue();
    const char * c_str() {
        return this->str_value.c_str();
    }
    void set_str(const char*s) {
        str_value = s;
    }
    int length();
    void set_str(const std::string & s) {
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
    std::string message_;
public:
    exception_type_t exception_type;
    ExceptionValue(exception_type_t e) : Value(VALUE_TYPE_EXCEPTION), exception_type(e) {
    }
    ExceptionValue(const char *format, ...);
    ExceptionValue(const std::string &msg) : Value(VALUE_TYPE_EXCEPTION), exception_type(EXCEPTION_TYPE_GENERAL) {
        message_ = msg;
    }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] exception(%d)\n", exception_type);
    }
    std::string message() {
        return message_;
    }
    const char *type_str() { return "exception"; }
};

};

#endif // VALUE_H_
