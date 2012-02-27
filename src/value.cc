#include "value.h"
#include "value/code.h"
#include "value/tuple.h"
#include <stdarg.h>

using namespace tora;

SharedPtr<BoolValue> Value::to_b() {
    switch (value_type) {
    case VALUE_TYPE_UNDEF: {
        return new BoolValue(false);
    }
    case VALUE_TYPE_BOOL:
        return this->upcast<BoolValue>();
    default: {
        return new BoolValue(true);
    }
    }
}

SharedPtr<StrValue> IntValue::to_s() {
    SharedPtr<StrValue> v = new StrValue();
    std::ostringstream os;
    os << this->upcast<IntValue>()->int_value;
    v->set_str(os.str());
    return v;
}

SharedPtr<StrValue> DoubleValue::to_s() {
    SharedPtr<StrValue> v = new StrValue();
    std::ostringstream os;
    os << this->double_value;
    v->set_str(os.str());
    return v;
}

SharedPtr<StrValue> BoolValue::to_s() {
    return new StrValue(this->bool_value ? "true" : "false");
}

SharedPtr<StrValue> UndefValue::to_s() {
    return new StrValue("undef"); // TODO
}
SharedPtr<StrValue> Value::to_s() {
    fprintf(stderr, "%s don't support stringification.\n", this->type_str());
    abort();
}

IntValue *Value::to_i() {
    if (value_type == VALUE_TYPE_INT) {
        IntValue *v = new IntValue(this->upcast<IntValue>()->int_value);
        // TODO: this->clone()?
        return v;
    } else if (value_type == VALUE_TYPE_TUPLE) {
        if (this->upcast<TupleValue>()->size() == 1) {
            return this->upcast<TupleValue>()->at(0)->to_i();
        }
    }


    printf("to_i is not supported yet in not VALUE_TYPE_INT(%d)\n", value_type);
    this->dump();
    abort();
}

SharedPtr<Value> IntValue::tora__neg__() {
    return new IntValue(-this->int_value);
}

ExceptionValue::ExceptionValue(const char *format, ...)
    : Value(VALUE_TYPE_EXCEPTION), exception_type(EXCEPTION_TYPE_GENERAL) {

    va_list ap;
    char p[4096+1];
    va_start(ap, format);
    vsnprintf(p, 4096, format, ap);
    va_end(ap);
    message_ = p;
}



