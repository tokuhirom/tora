#include "value.h"
#include "value/code.h"
#include "value/tuple.h"
#include "value/range.h"
#include "value/bool.h"
#include "value/double.h"
#include "value/object.h"
#include "value/symbol.h"
#include "value/bytes.h"
#include "symbols.gen.h"
#include "peek.h"
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
    case VALUE_TYPE_REFERENCE: return "Reference";
    case VALUE_TYPE_CLASS: return "Class";
    case VALUE_TYPE_FILE_PACKAGE: return "FilePackage";
    case VALUE_TYPE_OBJECT: {
        return ((const ObjectValue*)this)->type_str();
    }
    }
    abort();
}

bool Value::to_bool() const {
    switch (value_type) {
    case VALUE_TYPE_UNDEF:
        return false;
    case VALUE_TYPE_BOOL:
        return static_cast<const BoolValue*>(this)->bool_value();
    default:
        return true;
    }
}

double Value::to_double() const {
    switch (value_type) {
    case VALUE_TYPE_INT:
        return static_cast<double>(static_cast<const IntValue*>(this)->int_value());
    case VALUE_TYPE_DOUBLE:
        return static_cast<double>(static_cast<const DoubleValue*>(this)->double_value());
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
    case VALUE_TYPE_CLASS:
    case VALUE_TYPE_FILE_PACKAGE:
    case VALUE_TYPE_REFERENCE:
        throw new ExceptionValue("%s cannot support to convert double value.", this->type_str());
    }
    abort();
}

SharedPtr<StrValue> Value::to_s() {
    switch (value_type) {
    case VALUE_TYPE_STR:
        return static_cast<StrValue*>(this);
    case VALUE_TYPE_BYTES:
        // TODO: utf8 validation?
        return new StrValue(static_cast<BytesValue*>(this)->str_value());
    case VALUE_TYPE_INT: {
        std::ostringstream os;
        os << this->upcast<IntValue>()->int_value();
        return new StrValue(os.str());
    }
    case VALUE_TYPE_DOUBLE: {
        std::ostringstream os;
        os << this->upcast<DoubleValue>()->double_value();
        return new StrValue(os.str());
    }
    case VALUE_TYPE_BOOL: {
        return new StrValue(this->upcast<BoolValue>()->bool_value() ? "true" : "false");
    }
    case VALUE_TYPE_UNDEF: {
        return new StrValue("undef");
    }
    case VALUE_TYPE_EXCEPTION: {
        ExceptionValue * e = this->upcast<ExceptionValue>();
        return new StrValue(e->message());
    }
    case VALUE_TYPE_RANGE: {
        std::ostringstream os;
        os << this->upcast<RangeValue>()->left()->int_value();
        os << "..";
        os << this->upcast<RangeValue>()->right()->int_value();
        return new StrValue(os.str());
    }
    default: {
        throw new ExceptionValue("%s don't support stringification.", this->type_str());
    }
    }
}

int Value::to_int() const {
    if (value_type == VALUE_TYPE_INT) {
        return static_cast<const IntValue*>(this)->int_value();
    } else if (value_type == VALUE_TYPE_DOUBLE) {
        return static_cast<int>(this->to_double());
    } else if (value_type == VALUE_TYPE_TUPLE) {
        if (static_cast<const TupleValue*>(this)->size() == 1) {
            return static_cast<const TupleValue*>(this)->at(0)->to_int();
        } else {
            throw new ExceptionValue("Cannot coerce tuple to integer");
        }
    } else if (value_type == VALUE_TYPE_STR) {
        const StrValue *s = static_cast<const StrValue*>(this);
        set_errno(0);
        char *endptr = (char*)(s->str_value().c_str()+s->str_value().size());
        long ret = strtol(s->str_value().c_str(), &endptr, 10);
        if (get_errno() == 0) {
            return ret;
        } else if (get_errno() == EINVAL) {
            throw new ExceptionValue("String contains non numeric character: %s", s->str_value().c_str());
        } else if (get_errno() == ERANGE) {
            // try to the bigint?
            TODO();
        } else {
            throw new ErrnoExceptionValue(get_errno());
        }
    } else if (value_type == VALUE_TYPE_SYMBOL) {
        fprintf(stderr, "[BUG] DO NOT CALL to_int() method in symbol value\n");
        abort();
    }

    throw new ExceptionValue("to_int is not supported yet in %s\n", this->type_str());
}

Value& tora::Value::operator=(const Value&lhs) {
    switch (this->value_type) {
    case VALUE_TYPE_INT:
    case VALUE_TYPE_DOUBLE:
    case VALUE_TYPE_UNDEF:
        // primitive value does not needs deletion
        break;
    case VALUE_TYPE_STR:
        delete this->str_value_;
        this->str_value_ = NULL;
        break;
    case VALUE_TYPE_ARRAY:
        delete this->array_value_;
        this->array_value_ = NULL;
        break;
    default:
        printf("OOPS %s=%s\n", this->type_str(), lhs.type_str());
        abort();
    }

    switch (lhs.value_type) {
    case VALUE_TYPE_INT: {
        const IntValue *vp = static_cast<const IntValue*>(&lhs);
        this->value_type = VALUE_TYPE_INT;
        this->upcast<IntValue>()->int_value(vp->int_value());
        return *this;
    }
    case VALUE_TYPE_UNDEF: {
        this->value_type = VALUE_TYPE_UNDEF;
        return *this;
    }
    case VALUE_TYPE_STR: {
        this->value_type = lhs.value_type;
        // printf("# COPY!\n");
        this->str_value_ = new std::shared_ptr<std::string>(*(lhs.str_value_));
        return *this;
    }
    case VALUE_TYPE_ARRAY: {
        const ArrayValue *ap = static_cast<const ArrayValue*>(&lhs);
        this->value_type = lhs.value_type;
        this->array_value_ = new ArrayImpl();
        // this->array_value_->reset(ap->array_value_->get());
        *(this->array_value_) = *(ap->array_value_);
        return *this;
    }
    default: {
        printf("OOPS %s=%s\n", this->type_str(), lhs.type_str());
        abort();
    }
    }
    abort();
}

/**
 * Return the class id the value binded.
 */
ID Value::object_package_id() const {
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
    case VALUE_TYPE_ARRAY_ITERATOR:
        return SYMBOL_ARRAY_ITERATOR_CLASS;
    case VALUE_TYPE_HASH_ITERATOR:
        return SYMBOL_HASH_ITERATOR_CLASS;
    case VALUE_TYPE_RANGE_ITERATOR:
        return SYMBOL_RANGE_ITERATOR_CLASS;
    case VALUE_TYPE_REFERENCE:
        return SYMBOL_REFERENCE_CLASS;
    case VALUE_TYPE_CLASS:
        return SYMBOL_CLASS_CLASS;
    case VALUE_TYPE_FILE_PACKAGE:
        return SYMBOL_FILEPACKAGE_CLASS;
    case VALUE_TYPE_TUPLE:
        fprintf(stderr, "[BUG] You must not get a package name from tuple.\n");
        abort();
    case VALUE_TYPE_SYMBOL:
        return static_cast<const SymbolValue*>(this)->id();
    case VALUE_TYPE_OBJECT:
        return static_cast<const ObjectValue*>(this)->class_value()->name_id();
    case VALUE_TYPE_UNDEF:
        throw new ExceptionValue("Cannot get package name from undefined value.");
    }
    abort();
}

