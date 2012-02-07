#include "value.h"
#include "code.h"

using namespace tora;

SharedPtr<Value> Value::to_b() {
    switch (value_type) {
    case VALUE_TYPE_UNDEF: {
        return new BoolValue(false);
    }
    case VALUE_TYPE_BOOL:
        return this;
    default: {
        return new BoolValue(true);
    }
    }
}

SharedPtr<StrValue> IntValue::to_s() {
    StrValue *v = new StrValue();
    std::ostringstream os;
    os << this->upcast<IntValue>()->int_value;
    v->set_str(strdup(os.str().c_str()));
    return v;
}

SharedPtr<StrValue> RangeValue::to_s() {
    SharedPtr<StrValue> v = new StrValue();
    std::ostringstream os;
    os << this->left->int_value;
    os << "..";
    os << this->right->int_value;
    v->set_str(strdup(os.str().c_str()));
    return &(*(v));
}

SharedPtr<StrValue> DoubleValue::to_s() {
    StrValue *v = new StrValue();
    std::ostringstream os;
    os << this->double_value;
    v->set_str(strdup(os.str().c_str()));
    return v;
}

SharedPtr<StrValue> BoolValue::to_s() {
    return new StrValue(strdup(this->bool_value ? "true" : "false"));
}

SharedPtr<StrValue> UndefValue::to_s() {
    return new StrValue(strdup("undef"));
}
SharedPtr<StrValue> CodeValue::to_s() {
    return new StrValue(strdup("<code>"));
}

SharedPtr<StrValue> Value::to_s() {
    printf("%s don't support stringification.\n", this->type_str());
    abort();
}

IntValue *Value::to_i() {
    switch (value_type) {
    case VALUE_TYPE_INT: {
        IntValue *v = new IntValue(this->upcast<IntValue>()->int_value);
        // TODO: this->clone()?
        return v;
    }
    default:
        printf("to_i is not supported yet in not VALUE_TYPE_INT(%d)\n", value_type);
        this->dump();
        abort();
    }
}

Value *IntValue::tora__neg__() {
    return new IntValue(-this->int_value);
}

StrValue::~StrValue() {
}

