#include "value.h"

using namespace tora;

Value *Value::to_b() {
    switch (value_type) {
    case VALUE_TYPE_NIL: {
        Value *v = new Value();
        v->set_bool(false);
        return v;
    }
    case VALUE_TYPE_BOOL:
        this->retain();
        return this;
    default: {
        Value *v = new Value();
        v->set_bool(true);
        return v;
    }
    }
}

