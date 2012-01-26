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

/**
 * The value class
 */
class Value {
public:
    value_type_t value_type;
    union {
        int  int_value;
        bool bool_value;
        const char *str_value;
    } value;
    int refcnt; // reference count

    Value() {
        refcnt = 1;
    }
    ~Value() {
        if (value_type == VALUE_TYPE_STR) {
            delete [] value.str_value;
        }
    }
    void release() {
        --refcnt;
        if (refcnt == 0) {
            delete this;
        }
    }
    void retain() {
        ++refcnt;
    }
    void set_int(int i) {
        value.int_value = i;
        value_type = VALUE_TYPE_INT;
    }
    void set_bool(bool b) {
        value.bool_value = b;
        value_type = VALUE_TYPE_BOOL;
    }
    void set_str(const char *s) {
        // memory leak
        value.str_value = s;
        value_type = VALUE_TYPE_STR;
    }
    void dump() {
        switch (value_type) {
        case VALUE_TYPE_INT:
            printf("[dump] IV: %d\n", value.int_value);
            break;
        case VALUE_TYPE_BOOL:
            printf("[dump] bool: %s\n", value.bool_value ? "true" : "false");
            break;
        case VALUE_TYPE_STR:
            printf("[dump] str: %s\n", value.str_value);
            break;
        default:
            printf("[dump] unknown: %d\n", value_type);
            break;
        }
    }
    Value *to_s() {
        switch (value_type) {
        case VALUE_TYPE_INT: {
            Value *v = new Value();
            std::ostringstream os;
            os << this->value.int_value;
            v->set_str(strdup(os.str().c_str()));
            return v;
        }
        case VALUE_TYPE_STR: {
            Value *v = new Value();
            v->set_str(strdup(this->value.str_value));
            return v;
        }
        case VALUE_TYPE_BOOL: {
            Value *v = new Value();
            v->set_str(strdup(this->value.bool_value ? "true" : "false"));
            return v;
        }
        case VALUE_TYPE_NIL: {
            Value *v = new Value();
            v->set_str(strdup("nil"));
            return v;
        }
        default:
            printf("[BUG] unknown in to_s: %d\n", value_type);
            abort();
            break;
        }
    }
    Value *to_i() {
        switch (value_type) {
        case VALUE_TYPE_INT: {
            Value *v = new Value();
            v->set_int(this->value.int_value);
            return v;
        }
        default:
            abort();
        }
    }
    Value *to_b();
};

#endif // VALUE_H_
