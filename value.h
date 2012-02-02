#ifndef VALUE_H_
#define VALUE_H_

#include "tora.h"
#include "op.h"
#include "ops.gen.h"
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
    void dump();
    // TODO: rename to as_str
    StrValue *to_s();
    // TODO: rename to as_int
    IntValue *to_i();
    // TODO: rename to as_bool
    Value *to_b();
    bool is_numeric() {
        return this->value_type == VALUE_TYPE_INT;
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
};

class StrValue: public Value {
public:
    const char*str_value;
    StrValue(): Value() {
        this->value_type = VALUE_TYPE_STR;
        this->str_value = NULL;
    }
    ~StrValue() {
        delete [] str_value;
    }
    void set_str(const char*s) {
        str_value = s;
    }
    void dump() {
        printf("[dump] str: %s\n", str_value);
    }
};

class ArrayValue: public Value {
public:
    std::vector<Value*> *values;
    ArrayValue() : Value() {
        this->value_type = VALUE_TYPE_ARRAY;
        this->values = new std::vector<Value*>();
    }
    ~ArrayValue() {
        delete values;
    }
    // retain before push
    void push(Value *v) {
        this->values->push_back(v);
    }
    // release after pop by your hand
    Value* pop() {
        Value *v = this->values->back();
        this->values->pop_back();
        return v;
    }
    Value *at(int i) {
        return this->values->at(i);
    }
    void dump() {
        printf("[dump] array:\n");
        for (size_t i=0; i<values->size(); i++) {
            values->at(i)->dump();
        }
    }
};

class CodeValue: public Value {
public:
    const char *code_name;
    std::vector<std::string*> *code_params;
    std::vector<OP*> *code_opcodes;

    CodeValue(): Value() {
        this->value_type = VALUE_TYPE_CODE;
    }
    void dump() {
        printf("[dump] code: name: %s\n", this->code_name);
        for (size_t i=0; i<this->code_opcodes->size(); i++) {
            printf("    [%d] %s\n", i, opcode2name[this->code_opcodes->at(i)->op_type]);
        }
        printf("----------------\n");
    }
};

struct ValueDeleter {
    typedef Value pointer;
    void operator ()(Value* handle) {
        handle->release();
    }
};

typedef std::unique_ptr<Value, ValueDeleter> ValuePtr;

};

#endif // VALUE_H_
