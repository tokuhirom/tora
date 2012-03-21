#include "value.h"
#include "value/code.h"
#include "value/tuple.h"
#include "value/range.h"
#include "value/bool.h"
#include "value/double.h"
#include "value/object.h"
#include "value/symbol.h"
#include "symbols.gen.h"
#include <stdarg.h>
#include <errno.h>

using namespace tora;

const char * Value::type_str() const {
    switch (this->value_type) {
    case VALUE_TYPE_RANGE: return "Range";
    case VALUE_TYPE_BOOL: return "Bool";
    case VALUE_TYPE_EXCEPTION: return "Exception";
    case VALUE_TYPE_HASH: return "Hash";
    case VALUE_TYPE_DOUBLE: return "Double";
    case VALUE_TYPE_INT: return "Int";
    case VALUE_TYPE_STR: return "Str";
    case VALUE_TYPE_FILE: return "File";
    case VALUE_TYPE_CODE: return "Code";
    case VALUE_TYPE_TUPLE: return "Tuple";
    case VALUE_TYPE_POINTER: return "Pointer";
    case VALUE_TYPE_REGEXP: return "Regexp";
    case VALUE_TYPE_RANGE_ITERATOR: return "Range::Iterator";
    case VALUE_TYPE_ARRAY: return "Array";
    case VALUE_TYPE_UNDEF: return "Undef";
    case VALUE_TYPE_SYMBOL: return "Symbol";
    case VALUE_TYPE_HASH_ITERATOR: return "Hash::Iterator";
    case VALUE_TYPE_ARRAY_ITERATOR: return "Array::Iterator";
    case VALUE_TYPE_BYTES: return "Bytes";
    case VALUE_TYPE_OBJECT: {
        return ((const ObjectValue*)this)->type_str();
    }
    }
    abort();
}

bool Value::to_bool() {
    switch (value_type) {
    case VALUE_TYPE_UNDEF:
        return false;
    case VALUE_TYPE_BOOL:
        return static_cast<BoolValue*>(this)->bool_value;
    default:
        return true;
    }
}

double Value::to_double() {
    switch (value_type) {
    case VALUE_TYPE_INT:
        return static_cast<double>(static_cast<IntValue*>(this)->int_value());
    case VALUE_TYPE_DOUBLE:
        return static_cast<double>(static_cast<DoubleValue*>(this)->double_value());
    case VALUE_TYPE_OBJECT:
        TODO();
    case VALUE_TYPE_BOOL:
    case VALUE_TYPE_CODE:
    case VALUE_TYPE_ARRAY:
    case VALUE_TYPE_REGEXP:
    case VALUE_TYPE_TUPLE:
    case VALUE_TYPE_FILE:
    case VALUE_TYPE_SYMBOL:
    case VALUE_TYPE_ARRAY_ITERATOR:
    case VALUE_TYPE_RANGE_ITERATOR:
    case VALUE_TYPE_HASH_ITERATOR:
    case VALUE_TYPE_HASH:
    case VALUE_TYPE_POINTER:
    case VALUE_TYPE_UNDEF:
    case VALUE_TYPE_EXCEPTION:
    case VALUE_TYPE_RANGE:
    case VALUE_TYPE_STR:
    case VALUE_TYPE_BYTES:
        throw new ExceptionValue("%s cannot support to convert double value.", this->type_str());
    }
    abort();
}

SharedPtr<StrValue> Value::to_s() {
    switch (value_type) {
    case VALUE_TYPE_STR:
        return static_cast<StrValue*>(this);
    case VALUE_TYPE_INT: {
        SharedPtr<StrValue> v = new StrValue();
        std::ostringstream os;
        os << this->upcast<IntValue>()->int_value();
        v->set_str(os.str());
        return v;
    }
    case VALUE_TYPE_DOUBLE: {
        SharedPtr<StrValue> v = new StrValue();
        std::ostringstream os;
        os << this->upcast<DoubleValue>()->double_value();
        v->set_str(os.str());
        return v;
    }
    case VALUE_TYPE_BOOL: {
        return new StrValue(this->upcast<BoolValue>()->bool_value ? "true" : "false");
    }
    case VALUE_TYPE_UNDEF: {
        return new StrValue("undef");
    }
    case VALUE_TYPE_EXCEPTION: {
        ExceptionValue * e = this->upcast<ExceptionValue>();
        return new StrValue(e->message());
    }
    case VALUE_TYPE_RANGE: {
        SharedPtr<StrValue> v = new StrValue();
        std::ostringstream os;
        os << this->upcast<RangeValue>()->left->int_value();
        os << "..";
        os << this->upcast<RangeValue>()->right->int_value();
        v->set_str(os.str());
        return v;
    }
    default: {
        throw new ExceptionValue("%s don't support stringification.\n", this->type_str());
    }
    }
}

int Value::to_int() {
    if (value_type == VALUE_TYPE_INT) {
        return this->upcast<IntValue>()->int_value();
    } else if (value_type == VALUE_TYPE_DOUBLE) {
        return static_cast<int>(this->to_double());
    } else if (value_type == VALUE_TYPE_TUPLE) {
        if (this->upcast<TupleValue>()->size() == 1) {
            return this->upcast<TupleValue>()->at(0)->to_int();
        } else {
            throw new ExceptionValue("Cannot coerce tuple to integer");
        }
    } else if (value_type == VALUE_TYPE_STR) {
        StrValue *s = this->upcast<StrValue>();
        errno = 0;
        char *endptr = (char*)(s->str_value.c_str()+s->str_value.size());
        long ret = strtol(s->str_value.c_str(), &endptr, 10);
        if (errno == 0) {
            return ret;
        } else if (errno == EINVAL) {
            throw new ExceptionValue("String contains non numeric character: %s", s->str_value.c_str());
        } else if (errno == ERANGE) {
            // try to the bigint?
            TODO();
        } else {
            throw new ExceptionValue(errno);
        }
    }

    throw new ExceptionValue("to_i is not supported yet in %s\n", this->type_str());
}

Value& tora::Value::operator=(const Value&v) {
    assert(this->value_type == v.value_type);
    switch (v.value_type) {
    case VALUE_TYPE_INT: {
        const IntValue *vp = static_cast<const IntValue*>(&v);
        this->upcast<IntValue>()->int_value(vp->int_value());
        return *this;
    }
    default:
        abort();
    }
}

ID Value::object_package_id() {
    switch (value_type) {
    case VALUE_TYPE_STR:
        return SYMBOL_STRING_CLASS;
    case VALUE_TYPE_CODE:
        return SYMBOL_CODE_CLASS;
    case VALUE_TYPE_ARRAY:
        return SYMBOL_ARRAY_CLASS;
    case VALUE_TYPE_FILE:
        return SYMBOL_FILE_CLASS;
    case VALUE_TYPE_INT:
        return SYMBOL_INT_CLASS;
    case VALUE_TYPE_DOUBLE:
        return SYMBOL_DOUBLE_CLASS;
    case VALUE_TYPE_HASH:
        return SYMBOL_HASH_CLASS;
    case VALUE_TYPE_BYTES:
        return SYMBOL_BYTES_CLASS;
    case VALUE_TYPE_REGEXP:
        return SYMBOL_REGEXP_CLASS;
    case VALUE_TYPE_BOOL:
        return SYMBOL_BOOLEAN_CLASS;
    case VALUE_TYPE_RANGE:
        return SYMBOL_RANGE_CLASS;
    case VALUE_TYPE_POINTER:
        return SYMBOL_POINTER_CLASS;
    case VALUE_TYPE_EXCEPTION:
        return SYMBOL_EXCEPTION_CLASS;
    case VALUE_TYPE_SYMBOL:
        return this->upcast<SymbolValue>()->id;
    case VALUE_TYPE_OBJECT:
        return this->upcast<ObjectValue>()->package_id();
    case VALUE_TYPE_UNDEF:
        throw new ExceptionValue("Cannot get package name from undefined value.");
    }
    abort();
}

