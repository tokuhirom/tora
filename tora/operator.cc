#include "operator.h"
#include "value.h"
#include "value/hash.h"
#include "value/object.h"
#include "value/array.h"
#include "ops.gen.h"

using namespace tora;

/**
 * binary addition operator
 */
Value * tora::op_add(const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        if (rhs->value_type == VALUE_TYPE_DOUBLE) {
            // upgrade to double type
            return new DoubleValue(lhs->to_double() + rhs->to_double());
        } else {
            int i = rhs->to_int();
            return new IntValue(lhs->upcast<IntValue>()->int_value() + i);
        }
    } else if (lhs->value_type == VALUE_TYPE_STR) {
        SharedPtr<Value> s(rhs->to_s());
        return new StrValue(lhs->upcast<StrValue>()->str_value() + s->upcast<StrValue>()->str_value());
    } else if (lhs->value_type == VALUE_TYPE_BYTES) {
        SharedPtr<Value> s(rhs->to_s());
        return new BytesValue(lhs->upcast<BytesValue>()->str_value() + s->upcast<BytesValue>()->str_value());
    } else if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        return new DoubleValue(lhs->to_double() + rhs->to_double());
    } else {
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not numeric or string.\n", s->upcast<StrValue>()->str_value().c_str());
    }
    abort();
}

/**
 * subtract lhs and rhs.
 * return value must be return by caller.
 */
Value * tora::op_sub(const SharedPtr<Value>& lhs, const SharedPtr<Value> & rhs) {
    if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        return new DoubleValue(lhs->to_double() - rhs->to_double());
    } else if (lhs->value_type == VALUE_TYPE_INT) {
        if (rhs->value_type == VALUE_TYPE_DOUBLE) { // upgrade
            return new DoubleValue(lhs->to_double() - rhs->to_double());
        } else {
            return new IntValue(lhs->upcast<IntValue>()->int_value() - rhs->to_int());
        }
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not numeric. You cannot subtract.", s->upcast<StrValue>()->str_value().c_str());
    }
    abort();
}

Value * tora::op_div(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        double rhs_double = rhs->to_double();
        if (rhs_double == 0) {
            throw new ZeroDividedExceptionExceptionValue();
        }
        return new DoubleValue(lhs->to_double() / rhs_double);
    } else if (lhs->value_type == VALUE_TYPE_INT) {
        if (rhs->value_type == VALUE_TYPE_DOUBLE) { // upgrade
            double rhs_double = rhs->to_double();
            if (rhs_double == 0) {
                throw new ZeroDividedExceptionExceptionValue();
            }
            return new DoubleValue(lhs->to_double() / rhs_double);
        } else {
            double rhs_int = rhs->to_int();
            if (rhs_int == 0) {
                throw new ZeroDividedExceptionExceptionValue();
            }
            return new IntValue(lhs->upcast<IntValue>()->int_value() / rhs_int);
        }
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not numeric. You cannot divide.", s->upcast<StrValue>()->str_value().c_str());
    }
    abort();
}

Value * tora::op_mul(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        return new DoubleValue(lhs->to_double() * rhs->to_double());
    } else if (lhs->value_type == VALUE_TYPE_INT) {
        if (rhs->value_type == VALUE_TYPE_DOUBLE) { // upgrade
            return new DoubleValue(lhs->to_double() * rhs->to_double());
        } else {
            return new IntValue(lhs->upcast<IntValue>()->int_value() * rhs->upcast<IntValue>()->int_value());
        }
    } else if (lhs->value_type == VALUE_TYPE_STR) {
        std::ostringstream buf;
        int n = rhs->to_int();
        for (int i=0; i<n; i++) {
            buf << lhs->upcast<StrValue>()->str_value();
        }
        return new StrValue(buf.str());
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not numeric. You cannot multiply.", s->upcast<StrValue>()->str_value().c_str());
    }
    abort();
}

Value * tora::op_pow(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_DOUBLE) {
        return new DoubleValue(pow(lhs->to_double(), rhs->to_double()));
    } else if (lhs->value_type == VALUE_TYPE_INT) {
        if (rhs->value_type == VALUE_TYPE_DOUBLE) { // upgrade
            return new DoubleValue(pow(lhs->to_double(), rhs->to_double()));
        } else {
            return new IntValue((int)pow(lhs->to_double(), rhs->to_double()));
        }
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not numeric. You cannot pow.", s->upcast<StrValue>()->str_value().c_str());
    }
    abort();
}

Value * tora::op_bitand(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        return new IntValue(lhs->to_int() & rhs->to_int());
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot and.", s->upcast<StrValue>()->str_value().c_str());
    }
    abort();
}

Value * tora::op_bitor(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        return new IntValue(lhs->to_int() | rhs->to_int());
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot bit or.", s->upcast<StrValue>()->str_value().c_str());
    }
    abort();
}

Value * tora::op_bitxor(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        return new IntValue(lhs->to_int() ^ rhs->to_int());
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot xor.", s->upcast<StrValue>()->str_value().c_str());
    }
    abort();
}

Value * tora::op_bitlshift(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        return new IntValue(lhs->to_int() << rhs->to_int());
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot <<.", s->upcast<StrValue>()->str_value().c_str());
    }
    abort();
}

Value * tora::op_bitrshift(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        return new IntValue(lhs->to_int() >> rhs->to_int());
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot >>.", s->upcast<StrValue>()->str_value().c_str());
    }
    abort();
}

Value * tora::op_modulo(const SharedPtr<Value> &lhs, const SharedPtr<Value> &rhs) {
    if (lhs->value_type == VALUE_TYPE_INT) {
        double x = lhs->to_int();
        double y = rhs->to_int();
        return new IntValue(x-y*floor(x/y));
    } else { 
        SharedPtr<Value> s(lhs->to_s());
        throw new ExceptionValue("'%s' is not integer. You cannot >>.", s->upcast<StrValue>()->str_value().c_str());
    }
    abort();
}

Value* tora::op_unary_negative(const SharedPtr<Value> & v) {
    switch (v->value_type) {
    case VALUE_TYPE_INT:
        return new IntValue(-(v->upcast<IntValue>()->int_value()));
    case VALUE_TYPE_DOUBLE:
        return new DoubleValue(-(v->upcast<DoubleValue>()->double_value()));
    case VALUE_TYPE_OBJECT:
        TODO();
    default:
        throw new ExceptionValue("%s is not a numeric. You cannot apply unary negative operator.\n", v->type_str());
    }
}

SharedPtr<Value> tora::op_get_item(const SharedPtr<Value>& container, const SharedPtr<Value> & index) {
    switch (container->value_type) {
    case VALUE_TYPE_HASH:
        return container->upcast<HashValue>()->get_item(index);
    case VALUE_TYPE_ARRAY:
        return container->upcast<ArrayValue>()->get_item(index);
    case VALUE_TYPE_OBJECT:
        return container->upcast<ObjectValue>()->get_item(index);
    default:
        throw new ExceptionValue("%s is not a container.\n", container->type_str());
    }
}

template <class operationI, class operationD, class OperationS>
bool tora::cmpop(operationI operation_i, operationD operation_d, OperationS operation_s, const SharedPtr<Value>& lhs, const SharedPtr<Value>& rhs) {
#ifndef NDEBUG
        if (rhs->value_type == VALUE_TYPE_SYMBOL) {
            fprintf(stderr, "[BUG] Do not compare with symbol value.\n");
            abort();
        }
#endif
 
    switch (lhs->value_type) {
    case VALUE_TYPE_INT: {
        int ie2 = rhs->to_int();
        return operation_i(lhs->upcast<IntValue>()->int_value(), ie2);
    }
    case VALUE_TYPE_STR: {
        SharedPtr<Value> s2(rhs->to_s());
        return (operation_s(lhs->upcast<StrValue>()->str_value(), s2->upcast<StrValue>()->str_value()));
    }
    case VALUE_TYPE_BYTES: {
        if (rhs->value_type == VALUE_TYPE_BYTES) {
            return (operation_s(lhs->upcast<BytesValue>()->str_value(), rhs->upcast<BytesValue>()->str_value()));
        } else {
            return false;
        }
    }
    case VALUE_TYPE_DOUBLE: {
        switch (rhs->value_type) {
        case VALUE_TYPE_INT: {
            return (operation_d(lhs->upcast<DoubleValue>()->double_value(), (double)rhs->upcast<IntValue>()->int_value()));
        }
        case VALUE_TYPE_DOUBLE: {
            return (operation_d(lhs->upcast<DoubleValue>()->double_value(), rhs->upcast<DoubleValue>()->double_value()));
        }
        default: {
            TODO(); // throw exception
            abort();
        }
        }
        break;
    }
    case VALUE_TYPE_BOOL: {
        return lhs->upcast<BoolValue>()->bool_value() == rhs->to_bool();
    }
    case VALUE_TYPE_UNDEF: {
        return rhs->value_type == VALUE_TYPE_UNDEF;
    }
    default:
        // TODO: support object comparation
        std::string err ("UNKNOWN MATCHING PATTERN lhs = '");
        err += lhs->type_str();
        err += "'\n";
        throw new ExceptionValue(err);
    }
    abort();
}

template bool tora::cmpop(std::equal_to<int>, std::equal_to<double>, std::equal_to<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);
template bool tora::cmpop(std::greater<int>, std::greater<double>, std::greater<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);
template bool tora::cmpop(std::greater_equal<int>, std::greater_equal<double>, std::greater_equal<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);
template bool tora::cmpop(std::less<int>, std::less<double>, std::less<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);
template bool tora::cmpop(std::less_equal<int>, std::less_equal<double>, std::less_equal<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);
template bool tora::cmpop(std::not_equal_to<int>, std::not_equal_to<double>, std::not_equal_to<std::string>, const SharedPtr<Value>&, const SharedPtr<Value> &);

