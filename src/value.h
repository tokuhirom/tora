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

#include <boost/pool/object_pool.hpp>

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
} value_type_t;

typedef enum {
    EXCEPTION_TYPE_UNDEF,
    EXCEPTION_TYPE_STOP_ITERATION,
    EXCEPTION_TYPE_GENERAL,
    EXCEPTION_TYPE_ERRNO,
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
class Value {
    PRIM_DECL(Value);
protected:
    Value(value_type_t t) : refcnt(0), value_type(t) { }
    virtual ~Value() { }
    Value(const Value&) = delete;
public:
    value_type_t value_type;
    Value& operator=(const Value&v);

    void dump() { this->dump(0); }
    void dump(int indent);
    virtual void dump(const SharedPtr<SymbolTable> & symbol_table, int indent) {
        this->dump(indent);
    }
    SharedPtr<StrValue> to_s();
    int to_int();
    double to_double();
    bool to_bool();
    bool is_numeric() {
        return this->value_type == VALUE_TYPE_INT;
    }

    template<class Y>
    Y* upcast() {
        return static_cast<Y*>(this);
    }

    // GET type name in const char*
    const char *type_str();

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
    void tora__decr__() {
        this->int_value--;
    }
    void tora__incr__() {
        this->int_value++;
    }
    SharedPtr<IntValue> clone() {
        return new IntValue(this->int_value);
    }
public:
	void* operator new(size_t size) { return pool_.malloc(); }
	void operator delete(void* doomed, size_t) { pool_.free((IntValue*)doomed); }
private:
    static boost::object_pool<IntValue> pool_;
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
        printf("[dump] undef(refcnt: %d)\n", refcnt);
    }
    const char *type_str() { return "undef"; }
};

// This is singleton
class BoolValue: public Value {
public:
    BoolValue(bool b): Value(VALUE_TYPE_BOOL) {
        this->bool_value = b;
    }
    bool bool_value;
    static BoolValue* true_instance() {
        return new BoolValue(true);
    }
    static BoolValue* false_instance() {
        return new BoolValue(false);
    }
    static BoolValue* instance(bool b) {
        return b ? BoolValue::true_instance() : BoolValue::false_instance();
    }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] bool: %s\n", bool_value ? "true" : "false");
    }
    const char *type_str() { return "bool"; }
public:
	void* operator new(size_t size) { return pool_.malloc(); }
	void operator delete(void* doomed, size_t) { pool_.free((BoolValue*)doomed); }
private:
    static boost::object_pool<BoolValue> pool_;
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
        printf("[dump] str: %s(refcnt: %d)\n", str_value.c_str(), refcnt);
    }
    const char *type_str() { return "str"; }
};

class ExceptionValue : public Value {
    std::string message_;
    int errno_;
public:
    exception_type_t exception_type;
    ExceptionValue(const char *format, ...);
    ExceptionValue(const std::string &msg) : Value(VALUE_TYPE_EXCEPTION), errno_(0), exception_type(EXCEPTION_TYPE_GENERAL) {
        message_ = msg;
    }
    ExceptionValue(int err) : Value(VALUE_TYPE_EXCEPTION), errno_(err), exception_type(EXCEPTION_TYPE_ERRNO) { }
    int get_errno() { return this->errno_; }
    void dump(int indent) {
        print_indent(indent);
        printf("[dump] exception(%d)\n", exception_type);
    }
    std::string message() {
        if (errno_) {
            return std::string(strerror(this->errno_));
        } else {
            return message_;
        }
    }
    const char *type_str() { return "exception"; }
};

class StopIterationExceptionValue : public ExceptionValue {
public:
    StopIterationExceptionValue() : ExceptionValue("") {
        exception_type = EXCEPTION_TYPE_STOP_ITERATION;
    }
};

};

#endif // VALUE_H_
