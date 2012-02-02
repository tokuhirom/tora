#include "value.h"

using namespace tora;

// singleton values
UndefValue *UndefValue::undef_  = NULL;
BoolValue *BoolValue::true_  = NULL;
BoolValue *BoolValue::false_ = NULL;

Value *Value::to_b() {
    switch (value_type) {
    case VALUE_TYPE_UNDEF: {
        return BoolValue::false_instance();
    }
    case VALUE_TYPE_BOOL:
        this->retain();
        return this;
    default: {
        return BoolValue::true_instance();
    }
    }
}

void Value::dump() {
    switch (value_type) {
    case VALUE_TYPE_INT:
        this->to_int()->dump();
        break;
    case VALUE_TYPE_BOOL:
        this->to_bool()->dump();
        break;
    case VALUE_TYPE_STR:
        this->to_str()->dump();
        break;
    case VALUE_TYPE_CODE: {
        this->to_code()->dump();
        break;
    }
    case VALUE_TYPE_ARRAY: {
        this->to_array()->dump();
        break;
    }
    case VALUE_TYPE_UNDEF: {
        printf("[dump] undef\n");
        break;
    }
    default:
        printf("[dump] unknown: %d\n", value_type);
        break;
    }
}

Value *ArrayValue::get_item(Value *index) {
    IntValue *iv = index->to_i();
    int i = iv->int_value;
    iv->release();
    return this->at(i);
}

StrValue *Value::to_s() {
    switch (value_type) {
    case VALUE_TYPE_INT: {
        StrValue *v = new StrValue();
        std::ostringstream os;
        os << this->to_int()->int_value;
        v->set_str(strdup(os.str().c_str()));
        return v;
    }
    case VALUE_TYPE_STR: {
        StrValue *v = new StrValue();
        v->set_str(strdup(this->to_str()->str_value));
        return v;
    }
    case VALUE_TYPE_BOOL: {
        StrValue *v = new StrValue();
        v->set_str(strdup(this->to_bool()->bool_value ? "true" : "false"));
        return v;
    }
    case VALUE_TYPE_UNDEF: {
        StrValue *v = new StrValue();
        v->set_str(strdup("undef"));
        return v;
    }
    default:
        printf("[BUG] unknown in to_s: %d\n", value_type);
        abort();
        break;
    }
}

IntValue *Value::to_i() {
    switch (value_type) {
    case VALUE_TYPE_INT: {
        IntValue *v = new IntValue(this->to_int()->int_value);
        // TODO: this->clone()?
        return v;
    }
    default:
        printf("to_i is not supported yet in not VALUE_TYPE_INT(%d)\n", value_type);
        this->dump();
        abort();
    }
}

