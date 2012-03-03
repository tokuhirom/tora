#include "value.h"
#include "value/code.h"
#include "value/tuple.h"
#include <stdarg.h>
#include <errno.h>

using namespace tora;

bool Value::to_bool() {
    switch (value_type) {
    case VALUE_TYPE_UNDEF:
        return false;
    case VALUE_TYPE_BOOL:
        return this->upcast<BoolValue>()->bool_value;
    default:
        return true;
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
    if (this->value_type == VALUE_TYPE_EXCEPTION) {
        ExceptionValue * e = this->upcast<ExceptionValue>();
        return new StrValue(e->message());
    } else {
        fprintf(stderr, "%s don't support stringification.\n", this->type_str());
        abort();
    }
}

Value *Value::to_int() {
    if (value_type == VALUE_TYPE_INT) {
        // IntValue *v = new IntValue(this->upcast<IntValue>()->int_value);
        return this;
    } else if (value_type == VALUE_TYPE_TUPLE) {
        if (this->upcast<TupleValue>()->size() == 1) {
            return this->upcast<TupleValue>()->at(0)->to_int();
        }
    } else if (value_type == VALUE_TYPE_STR) {
        StrValue *s = this->upcast<StrValue>();
        errno = 0;
        char *endptr = (char*)(s->str_value.c_str()+s->str_value.size());
        long ret = strtol(s->str_value.c_str(), &endptr, 10);
        if (errno == 0) {
            return new IntValue(ret);
        } else if (errno == EINVAL) {
            return new ExceptionValue("String contains non numeric character: %s", s->str_value.c_str());
        } else if (errno == ERANGE) {
            // try to the bigint?
            TODO();
        } else {
            return new ExceptionValue("Unknown error in strtol: %s(%d)", s->str_value.c_str(), errno);
        }
    }

    return new ExceptionValue("to_i is not supported yet in %s\n", this->type_str());
}

ExceptionValue::ExceptionValue(const char *format, ...)
    : Value(VALUE_TYPE_EXCEPTION), errno_(0), exception_type(EXCEPTION_TYPE_GENERAL) {

    va_list ap;
    char p[4096+1];
    va_start(ap, format);
    vsnprintf(p, 4096, format, ap);
    va_end(ap);
    message_ = p;
}

Value& tora::Value::operator=(const Value&v) {
    assert(this->value_type == v.value_type);
    switch (v.value_type) {
    case VALUE_TYPE_INT: {
        const IntValue *vp = static_cast<const IntValue*>(&v);
        this->upcast<IntValue>()->int_value = vp->int_value;
        return *this;
    }
    default:
        abort();
    }
}

