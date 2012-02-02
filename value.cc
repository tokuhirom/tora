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

Value *ArrayValue::get_item(Value *index) {
    IntValue *iv = index->to_i();
    int i = iv->int_value;
    iv->release();
    return this->at(i);
}

StrValue *IntValue::to_s() {
    StrValue *v = new StrValue();
    std::ostringstream os;
    os << this->to_int()->int_value;
    v->set_str(strdup(os.str().c_str()));
    return v;
}

StrValue *BoolValue::to_s() {
    return new StrValue(strdup(this->bool_value ? "true" : "false"));
}

StrValue *UndefValue::to_s() {
    return new StrValue(strdup("undef"));
}
StrValue *CodeValue::to_s() {
    return new StrValue(strdup("<code>"));
}

StrValue *Value::to_s() {
    printf("%s don't support stringification.\n", this->type_str());
    abort();
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

